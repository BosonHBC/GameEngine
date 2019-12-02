#include "cGeometryBuilder.h"
#include "Engine/Platform/Platform.h"
#include "Tools/AssetBuildLibrary/Functions.h"
#include "External/Lua/Includes.h"
#include <Engine/ScopeGuard/cScopeGuard.h>
#include "Engine/Graphics/VertexFormats.h"
#include <fstream>

eae6320::cResult eae6320::Assets::cGeometryBuilder::Build(const std::vector<std::string>& i_arguments)
{
	auto Result = Results::Success;
	std::string errorMessage;

	// Load Human-readable data from Lua
	uint8_t o_leftHand;
	uint16_t o_vertexCount;
	eae6320::Graphics::VertexFormats::s3dObject* o_vertexes;
	uint16_t o_indexCount;
	uint16_t* o_indices;
	if (!(Result = Load(m_path_source, o_leftHand, o_vertexCount, o_vertexes, o_indexCount, o_indices))) {
		OutputErrorMessageWithFileInfo(m_path_source, errorMessage.c_str());
	}
	FixIndexData(o_indices, o_indexCount, o_leftHand);
	std::ofstream outfile(m_path_target, std::ofstream::binary);
	if (outfile.is_open()) {
		outfile.write(reinterpret_cast<const char*>(&o_vertexCount), sizeof(uint16_t));
		outfile.write(reinterpret_cast<const char*>(o_vertexes), sizeof(eae6320::Graphics::VertexFormats::s3dObject) * o_vertexCount);
		outfile.write(reinterpret_cast<const char*>(&o_indexCount), sizeof(uint16_t));
		outfile.write(reinterpret_cast<const char*>(o_indices), sizeof(uint16_t) * o_indexCount);
		outfile.close();
	}


	delete o_vertexes;
	delete o_indices;
	return Result;
}

eae6320::cResult eae6320::Assets::cGeometryBuilder::Load(const std::string i_path, uint8_t& o_leftHand, uint16_t& o_vertexCount, eae6320::Graphics::VertexFormats::s3dObject*& o_vertexes, uint16_t& o_indexCount, uint16_t*& o_indices)
{

	auto result = Results::Success;

	// Create a new Lua state
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

	// Load the asset file as a "chunk",
		// meaning there will be a callable function at the top of the stack
	const auto stackTopBeforeLoad = lua_gettop(luaState);
	// Initialize Lua state

	{
		const auto luaResult = luaL_loadfile(luaState, i_path.c_str());
		if (luaResult != LUA_OK)
		{
			result = eae6320::Results::Failure;
			EAE6320_ASSERTF(false, lua_tostring(luaState, -1));
			// Pop the error message
			lua_pop(luaState, 1);
			return result;
		}
	}
	// Execute the "chunk", which should load the asset
	// into a table at the top of the stack
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
				// Pop every value that was returned
				lua_pop(luaState, returnedValueCount);
				return result;
			}
		}
		else
		{
			result = eae6320::Results::InvalidFile;
			EAE6320_ASSERTF(false, lua_tostring(luaState, -1));
			// Pop the error message
			lua_pop(luaState, 1);
			return result;
		}
	}
	// If this code is reached the asset file was loaded successfully,
	// and its table is now at index -1
	eae6320::cScopeGuard scopeGuard_popAssetTable([luaState]
		{
			lua_pop(luaState, 1);
		});

	// From now on, can load the data
	lua_State& io_luaState = *luaState;

	bool leftHanded = false;

	constexpr auto* const leftHandKey = "leftHand";
	lua_pushstring(&io_luaState, leftHandKey);
	lua_gettable(&io_luaState, -2);

	if (lua_isboolean(&io_luaState, -1))
	{
		leftHanded = (bool)lua_toboolean(&io_luaState, -1);
	}
	lua_pop(&io_luaState, 1);

	o_leftHand = (uint8_t)leftHanded;

	//
	// Loading Vertex Data
	//

	eae6320::Graphics::VertexFormats::s3dObject* vertexData;
	unsigned int vertexesCount;

	constexpr auto* const vertexKey = "vertexes";
	lua_pushstring(&io_luaState, vertexKey);
	lua_gettable(&io_luaState, -2);


	if (lua_istable(&io_luaState, -1))
	{
		vertexesCount = (unsigned int)luaL_len(&io_luaState, -1);
		vertexData = new eae6320::Graphics::VertexFormats::s3dObject[vertexesCount];

		for (unsigned int i = 1; i <= vertexesCount; ++i) {
			lua_pushinteger(&io_luaState, i);
			lua_gettable(&io_luaState, -2);
			// Load x
			{
				lua_pushinteger(&io_luaState, 1);
				lua_gettable(&io_luaState, -2);
				// Get Data here
				vertexData[i - 1].x = (float)lua_tonumber(&io_luaState, -1);
				lua_pop(&io_luaState, 1);
			}
			// Load y
			{
				lua_pushinteger(&io_luaState, 2);
				lua_gettable(&io_luaState, -2);
				// Get Data here
				vertexData[i - 1].y = (float)lua_tonumber(&io_luaState, -1);
				lua_pop(&io_luaState, 1);
			}
			// load z
			{
				lua_pushinteger(&io_luaState, 3);
				lua_gettable(&io_luaState, -2);
				// Get Data here
				vertexData[i - 1].z = (float)lua_tonumber(&io_luaState, -1);
				lua_pop(&io_luaState, 1);
			}

			lua_pop(&io_luaState, 1);
		}

	}
	else
	{
		result = eae6320::Results::InvalidFile;
		EAE6320_ASSERTF(false, "The value must be a table");
		return result;
	}
	// finish load vertex data
	lua_pop(&io_luaState, 1);

	o_vertexes = vertexData;
	o_vertexCount = vertexesCount;
	//
	// Loading Index Data
	//
	uint16_t* indexData;
	uint16_t indexCount;

	constexpr auto* const indexKey = "indices";
	lua_pushstring(&io_luaState, indexKey);
	lua_gettable(&io_luaState, -2);

	if (lua_istable(&io_luaState, -1))
	{
		indexCount = (uint16_t)luaL_len(&io_luaState, -1);
		indexData = new uint16_t[indexCount];
		for (uint16_t i = 1; i <= indexCount; ++i)
		{
			lua_pushinteger(&io_luaState, i);
			lua_gettable(&io_luaState, -2);
			// Get Data here
			indexData[i - 1] = (uint16_t)lua_tonumber(&io_luaState, -1);
			lua_pop(&io_luaState, 1);
		}
	}
	else
	{
		result = eae6320::Results::InvalidFile;
		EAE6320_ASSERTF(false, "The value must be a table");
		return result;
	}
	// finish load index data
	lua_pop(&io_luaState, 1);

	o_indices = indexData;
	o_indexCount = indexCount;

	return result;
}

void eae6320::Assets::cGeometryBuilder::FixIndexData(uint16_t*& o_indicesData, uint16_t i_indexCount, bool i_useLeftHand)
{
	uint16_t triangleCount = i_indexCount / 3;
	for (uint16_t i = 0; i < triangleCount; i++)
	{

#if defined( EAE6320_PLATFORM_D3D )
		if (i_useLeftHand) {
			uint16_t temp = o_indicesData[i * 3];
			o_indicesData[i * 3] = o_indicesData[i * 3 + 1];
			o_indicesData[i * 3 + 1] = temp;
		}
#elif defined( EAE6320_PLATFORM_GL )
		if (!i_useLeftHand) {
			uint16_t temp = o_indicesData[i * 3];
			o_indicesData[i * 3] = o_indicesData[i * 3 + 1];
			o_indicesData[i * 3 + 1] = temp;
		}
#endif
	}
}
