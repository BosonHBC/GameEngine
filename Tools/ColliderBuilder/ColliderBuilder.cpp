#include "ColliderBuilder.h"

#include <Engine/Graphics/cEffect.h>
#include <Tools/AssetBuildLibrary/Functions.h>
#include <Engine/Platform/Platform.h>
#include <Engine/ScopeGuard/cScopeGuard.h>
#include <External/Lua/Includes.h>
#include <fstream>
// Inherited Implementation
//=========================

// Build
//------

eae6320::cResult PlutoShe::Assets::ColliderBuilder::Build(const std::vector<std::string>& i_arguments)
{
	auto result = eae6320::Results::Success;
	std::string errorMessage;
	PlutoShe::Physics::Collider loadCollider;
	InitData(m_path_source, loadCollider);
	std::ofstream outfile(m_path_target, std::ofstream::binary);
	if (!outfile.is_open())
	{
		eae6320::Assets::OutputErrorMessageWithFileInfo(m_path_source, "no invalid geometry output path!");

		result = eae6320::Results::Failure;
		return result;
	}
	uint16_t vertexNum = static_cast<uint16_t>(loadCollider.m_vertices.size());

	outfile.write(reinterpret_cast<const char*>(&vertexNum), sizeof(vertexNum));
	outfile.write(reinterpret_cast<const char*>(&loadCollider.m_vertices[0]), vertexNum * sizeof(PlutoShe::Physics::Vector3));

	outfile.close();
	return result;
}



eae6320::cResult PlutoShe::Assets::ColliderBuilder::LoadVerticesFromLua(lua_State& io_luaState, PlutoShe::Physics::Collider& t_collider)
{
	auto result = eae6320::Results::Success;

	constexpr auto* const keyVertices = "vertices";
	lua_pushstring(&io_luaState, keyVertices);
	lua_gettable(&io_luaState, -2);
	eae6320::cScopeGuard scopeGuard_popTexturePath([&io_luaState]
	{
		lua_pop(&io_luaState, 1);
	});

	if (lua_istable(&io_luaState, -1))
	{
		const auto verticesCount = luaL_len(&io_luaState, -1);
		for (int i = 1; i <= verticesCount; ++i)
		{
			lua_pushinteger(&io_luaState, i);
			lua_gettable(&io_luaState, -2);
			eae6320::cScopeGuard scopeGuard_popVerticesTable([&io_luaState]
			{
				lua_pop(&io_luaState, 1);
			});
			PlutoShe::Physics::Vector3 currentVertex;
			if (lua_istable(&io_luaState, -1))
			{
				{
					lua_pushstring(&io_luaState, "position");
					lua_gettable(&io_luaState, -2);
					eae6320::cScopeGuard scopeGuard_popVerticesPosition([&io_luaState]
					{
						lua_pop(&io_luaState, 1);
					});
					if (lua_istable(&io_luaState, -1))
					{
						for (int j = 1; j <= 3; j++)
						{
							lua_pushinteger(&io_luaState, j);
							auto result = lua_gettable(&io_luaState, -2);
							auto currentValue = static_cast<float>(lua_tonumber(&io_luaState, -1));
							eae6320::cScopeGuard scopeGuard_popVertexPos([&io_luaState]
							{
								lua_pop(&io_luaState, 1);
							});
							if (result > 0)
							{
								switch (j)
								{
								case 1: currentVertex.m_x = currentValue; break;
								case 2: currentVertex.m_y = currentValue; break;
								case 3: currentVertex.m_z = currentValue; break;
								}
							}
						}
					}
				}
			
				t_collider.m_vertices.push_back(currentVertex);
			}

		}
	}
	else {
		result = eae6320::Results::InvalidFile;
		EAE6320_ASSERTF(false, "No vertices table");
		return result;
	}
	return result;
}



eae6320::cResult PlutoShe::Assets::ColliderBuilder::InitData(std::string i_path, PlutoShe::Physics::Collider& t_collider)
{
	auto result = eae6320::Results::Success;
	lua_State* luaState = nullptr;
	eae6320::cScopeGuard scopeGuard_onExit([&luaState]
	{
		if (luaState)
		{
			// If I haven't made any mistakes
			// there shouldn't be anything on the stack
			// regardless of any errors
			EAE6320_ASSERT(lua_gettop(luaState) == 0);

			lua_close(luaState);
			luaState = nullptr;
		}
	});

	{
		luaState = luaL_newstate();
		if (!luaState)
		{
			result = eae6320::Results::OutOfMemory;
			EAE6320_ASSERTF(false, "Failed to create a new Lua state");
			return result;
		}
	}
	const auto stackTopBeforeLoad = lua_gettop(luaState);
	{
		const auto luaResult = luaL_loadfile(luaState, i_path.c_str());
		if (luaResult != LUA_OK)
		{
			result = eae6320::Results::Failure;
			auto errorMessage = lua_tostring(luaState, -1);
			EAE6320_ASSERTF(false, "cannot load file");
			// Pop the error message
			lua_pop(luaState, 1);
			return result;
		}
	}

	{
		constexpr int argumentCount = 0;
		constexpr int returnValueCount = LUA_MULTRET;	// Return _everything_ that the file returns
		constexpr int noMessageHandler = 0;
		const auto luaResult = lua_pcall(luaState, argumentCount, returnValueCount, noMessageHandler);
		if (luaResult == LUA_OK)
		{
			// A well-behaved asset file will only return a single value
			const auto returnedValueCount = lua_gettop(luaState) - stackTopBeforeLoad;
			if (returnedValueCount == 1)
			{
				// A correct asset file _must_ return a table
				if (!lua_istable(luaState, -1))
				{
					result = eae6320::Results::InvalidFile;
					//std::string errorMessage = "Asset files must return a table (instead of a " + luaL_typename(luaState, -1) + ")";
					EAE6320_ASSERTF(false, "Asset files must return a table");
					// Pop the returned non-table value
					lua_pop(luaState, 1);
					return result;
				}
			}
			else
			{
				result = eae6320::Results::InvalidFile;
				EAE6320_ASSERTF(false, "Asset files must return a single table");
				/*std::cerr << "Asset files must return a single table (instead of "
					<< returnedValueCount << " values)" << std::endl;*/
					// Pop every value that was returned
				lua_pop(luaState, returnedValueCount);
				return result;
			}
		}
		else
		{
			result = eae6320::Results::InvalidFile;
			EAE6320_ASSERTF(false, "Read file failed");
			//std::cerr << lua_tostring(luaState, -1);
			// Pop the error message
			lua_pop(luaState, 1);
			return result;
		}
	}

	// If this code is reached the asset file was loaded successfully,
	// and its table is now at index -1

	// A scope guard is used to pop the asset as soon as the scope (this function) is left
	eae6320::cScopeGuard scopeGuard_popAssetTable([luaState]
	{
		lua_pop(luaState, 1);
	});

	if (!(result = LoadVerticesFromLua(*luaState, t_collider)))
	{
		return result;
	}
	return result;
}