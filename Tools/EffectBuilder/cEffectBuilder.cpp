#include "cEffectBuilder.h"
#include "Engine/Platform/Platform.h"
#include "Tools/AssetBuildLibrary/Functions.h"
#include "External/Lua/Includes.h"
#include <Engine/ScopeGuard/cScopeGuard.h>
#include "Engine/Graphics/cRenderState.h"
#include <fstream>

eae6320::cResult eae6320::Assets::cEffectBuilder::Build(const std::vector<std::string>& i_arguments)
{
	auto result = Results::Success;
	std::string errorMessage;
	// Load Human-readable data from Lua
	std::string o_vertexShaderPath;
	std::string o_fragmentShaderPath;
	uint8_t* o_renderState;
	if (!(result = Load(m_path_source, o_vertexShaderPath, o_fragmentShaderPath, o_renderState))) {
		OutputErrorMessageWithFileInfo(m_path_source, errorMessage.c_str());
	}

	uint8_t defaultRenderState = 0;
	o_renderState[0]?eae6320::Graphics::RenderStates::EnableAlphaTransparency(defaultRenderState): eae6320::Graphics::RenderStates::DisableAlphaTransparency(defaultRenderState);
	o_renderState[1] ? eae6320::Graphics::RenderStates::EnableDepthTesting(defaultRenderState) : eae6320::Graphics::RenderStates::DisableDepthTesting(defaultRenderState);
	o_renderState[2] ? eae6320::Graphics::RenderStates::EnableDepthWriting(defaultRenderState) : eae6320::Graphics::RenderStates::DisableDepthWriting(defaultRenderState);
	o_renderState[3] ? eae6320::Graphics::RenderStates::EnableDrawingBothTriangleSides(defaultRenderState) : eae6320::Graphics::RenderStates::DisableDrawingBothTriangleSides(defaultRenderState);

	std::string pathPrefix = "data/";
	std::string NULL_termination = "\0";

	o_vertexShaderPath = pathPrefix + o_vertexShaderPath + NULL_termination;
	o_fragmentShaderPath = pathPrefix + o_fragmentShaderPath + NULL_termination;
	const char* str_vertexPath = o_vertexShaderPath.c_str();
	size_t str_vertexPath_len = o_vertexShaderPath.size() + 1;
	const char* str_fragmentPath = o_fragmentShaderPath.c_str();
	size_t str_fragmentPath_len = o_fragmentShaderPath.size() + 1;

	std::ofstream outfile(m_path_target, std::ofstream::binary);
	if (outfile.is_open()) {
		outfile.write(reinterpret_cast<const char*>(&defaultRenderState), sizeof(uint8_t));
		outfile.write(reinterpret_cast<const char*>(str_vertexPath), str_vertexPath_len);
		outfile.write(reinterpret_cast<const char*>(str_fragmentPath),  str_fragmentPath_len);
		outfile.close();
	}

	return result;
}

eae6320::cResult eae6320::Assets::cEffectBuilder::Load(const std::string i_path, std::string& o_vertexShaderPath, std::string& o_fragmentShaderPath, uint8_t*& o_renderState)
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

	// Load vertex shader path
	{
		std::string vertexShaderPath;
		constexpr auto* const vertexShaderPathKey = "vertexShaderPath";
		lua_pushstring(&io_luaState, vertexShaderPathKey);
		lua_gettable(&io_luaState, -2);

		if (lua_isstring(&io_luaState, -1))
		{
			vertexShaderPath = (std::string)lua_tostring(&io_luaState, -1);
		}
		else {
			result = Results::Failure;
		}
		lua_pop(&io_luaState, 1);
		o_vertexShaderPath = vertexShaderPath;
	}
	// Load fragment shader path
	{
		std::string fragmentShaderPath;
		constexpr auto* const fragmentShaderPathKey = "fragmentShaderPath";
		lua_pushstring(&io_luaState, fragmentShaderPathKey);
		lua_gettable(&io_luaState, -2);

		if (lua_isstring(&io_luaState, -1))
		{
			fragmentShaderPath = (std::string)lua_tostring(&io_luaState, -1);
		}
		else {
			result = Results::Failure;
		}
		lua_pop(&io_luaState, 1);
		o_fragmentShaderPath = fragmentShaderPath;
	}

	//
	// Load RenderState
	//
	uint8_t* renderStateBools;

	constexpr auto* const indexKey = "renderState";
	lua_pushstring(&io_luaState, indexKey);
	lua_gettable(&io_luaState, -2);

	if (lua_istable(&io_luaState, -1))
	{
		renderStateBools = new uint8_t[4];
		lua_pushstring(&io_luaState, "IsAlphaTransparencyEnabled");
		lua_gettable(&io_luaState, -2);
		renderStateBools[0] = (uint8_t)lua_toboolean(&io_luaState, -1);
		lua_pop(&io_luaState, 1);

		lua_pushstring(&io_luaState, "IsDepthTestingEnabled");
		lua_gettable(&io_luaState, -2);
		renderStateBools[1] = (uint8_t)lua_toboolean(&io_luaState, -1);
		lua_pop(&io_luaState, 1);

		lua_pushstring(&io_luaState, "IsDepthWritingEnabled");
		lua_gettable(&io_luaState, -2);
		renderStateBools[2] = (uint8_t)lua_toboolean(&io_luaState, -1);
		lua_pop(&io_luaState, 1);

		lua_pushstring(&io_luaState, "ShouldBothTriangleSidesBeDrawn");
		lua_gettable(&io_luaState, -2);
		renderStateBools[3] = (uint8_t)lua_toboolean(&io_luaState, -1);
		lua_pop(&io_luaState, 1);

	}
	else
	{
		result = eae6320::Results::InvalidFile;
		EAE6320_ASSERTF(false, "The value must be a table");
		return result;
	}
	// finish load RenderState
	lua_pop(&io_luaState, 1);
	o_renderState = renderStateBools;
	
	return result;
}
