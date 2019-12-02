#include "cInputBuilder.h"
#include "Engine/Platform/Platform.h"
#include "Tools/AssetBuildLibrary/Functions.h"
#include "External/Lua/Includes.h"
#include <Engine/ScopeGuard/cScopeGuard.h>
#include "Engine/Asserts/Asserts.h"
#include <fstream>

/**
* Reference:
* https://docs.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
*/
typedef  eae6320::UserInput::KeyCodes::eKeyCodes eKeyCodes;
std::map<std::string, eKeyCodes> s_keyCodeMaps =
{
	{"Left", eKeyCodes::Left},
	{"Up", eKeyCodes::Up},
	{"Right", eKeyCodes::Right},
	{"Down", eKeyCodes::Down},
	{"Space", eKeyCodes::Space},
	{"Escape", eKeyCodes::Escape},
	{"Shift", eKeyCodes::Shift},
	{"Control", eKeyCodes::Control},
	{"Alt", eKeyCodes::Alt},
	{"Tab", eKeyCodes::Tab},
	{"CapsLock", eKeyCodes::CapsLock},
	{"BackSpace", eKeyCodes::BackSpace},
	{"Enter", eKeyCodes::Enter},
	{"Delete", eKeyCodes::Delete},
	{"PageUp", eKeyCodes::PageUp},
	{"PageDown", eKeyCodes::PageDown},
	{"End", eKeyCodes::End},
	{"Home", eKeyCodes::Home},
	{"F1", eKeyCodes::F1},
	{"F2", eKeyCodes::F2},
	{"F3", eKeyCodes::F3},
	{"F4", eKeyCodes::F4},
	{"F5", eKeyCodes::F5},
	{"F6", eKeyCodes::F6},
	{"F7", eKeyCodes::F7},
	{"F8", eKeyCodes::F8},
	{"F9", eKeyCodes::F9},
	{"F10", eKeyCodes::F10},
	{"F11", eKeyCodes::F11},
	{"F12", eKeyCodes::F12},
};


eae6320::cResult eae6320::Assets::cInputBuilder::Build(const std::vector<std::string>& i_arguments)
{
	auto result = Results::Success;
	std::string errorMessage;
	std::map<std::string, std::string> o_actionMappings;
	std::map<std::string, FAxisBrother> o_axisMappings;
	uint8_t o_actionMappingCount = 0;
	uint8_t o_axisMappingCount = 0;
	if (!(result = Load(m_path_source, o_actionMappings, o_actionMappingCount, o_axisMappings, o_axisMappingCount))) {
		OutputErrorMessageWithFileInfo(m_path_source, errorMessage.c_str());
	}

	std::string NULL_termination = "\0";
	std::ofstream outfile(m_path_target, std::ofstream::binary);

	if (outfile.is_open()) {
		outfile.write(reinterpret_cast<const char*>(&o_actionMappingCount), sizeof(uint8_t));
		outfile.write(reinterpret_cast<const char*>(&o_axisMappingCount), sizeof(uint8_t));
		for (auto it = o_actionMappings.begin(); it != o_actionMappings.end(); ++it)
		{
			std::string actionName = (it->first + NULL_termination);
			outfile.write(reinterpret_cast<const char*>(actionName.c_str()), actionName.size() + 1);
			std::string actionKey = it->second;
			eKeyCodes o_outKeyCode;

			if(!ConvertKeyFromStringToKeyCode(actionKey, o_outKeyCode)){
				EAE6320_ASSERTF(false, "There is invalid key: [%s] in the input sheet!", actionKey.c_str());
				result = Results::Failure;

			}
			outfile.write(reinterpret_cast<const char*>(&o_outKeyCode), sizeof(eKeyCodes));
		}

		for (auto it = o_axisMappings.begin(); it != o_axisMappings.end(); ++it)
		{
			std::string axisName = (it->first + NULL_termination);
			outfile.write(reinterpret_cast<const char*>(axisName.c_str()), axisName.size() + 1);

			// Write negative key code
			{
				std::string negativeKey = it->second.negative_key;
				eKeyCodes o_outNegativeKeyCode;

				if (!ConvertKeyFromStringToKeyCode(negativeKey, o_outNegativeKeyCode)) {
					EAE6320_ASSERTF(false, "There is invalid key: [%s] in the input sheet!", negativeKey.c_str());
					result = Results::Failure;

				}
				outfile.write(reinterpret_cast<const char*>(&o_outNegativeKeyCode), sizeof(eKeyCodes));
			}

			// Write positive key code
			{
				std::string positiveKey = it->second.positive_key;
				eKeyCodes o_outPositiveKeyCode;
				if (!ConvertKeyFromStringToKeyCode(positiveKey, o_outPositiveKeyCode)) {
					EAE6320_ASSERTF(false, "There is invalid key: [%s] in the input sheet!", positiveKey.c_str());
					result = Results::Failure;
				}
				outfile.write(reinterpret_cast<const char*>(&o_outPositiveKeyCode), sizeof(eKeyCodes));
			}

		}
		outfile.close();
	}

	return result;
}

eae6320::cResult eae6320::Assets::cInputBuilder::Load(const std::string i_path, std::map<std::string, std::string>& o_actionMappings, uint8_t& o_actionMappingCount, std::map<std::string, FAxisBrother>& o_axisMappings, uint8_t& o_axisMappingCount)
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

	// Load ActionMappings
	{
		constexpr auto* const actionMappingKey = "ActionMappings";
		lua_pushstring(&io_luaState, actionMappingKey);
		lua_gettable(&io_luaState, -2);

		if (lua_istable(&io_luaState, -1))
		{
			o_actionMappingCount = (uint8_t)luaL_len(&io_luaState, -1);

			for (uint8_t i = 1; i <= o_actionMappingCount; ++i)
			{
				lua_pushinteger(&io_luaState, i);
				lua_gettable(&io_luaState, -2);
				// Get Data here
				std::string actionName, keyName;
				{
					constexpr auto* const ActionNameKey = "ActionName";
					lua_pushstring(&io_luaState, ActionNameKey);
					lua_gettable(&io_luaState, -2);
					actionName = (std::string)lua_tostring(&io_luaState, -1);
					lua_pop(&io_luaState, 1);
				}
				{
					constexpr auto* const ActionNameKey = "Key";
					lua_pushstring(&io_luaState, ActionNameKey);
					lua_gettable(&io_luaState, -2);
					keyName = (std::string)lua_tostring(&io_luaState, -1);
					lua_pop(&io_luaState, 1);
				}
				o_actionMappings.insert(std::pair<std::string, std::string>(actionName, keyName));
				lua_pop(&io_luaState, 1);
			}
		}
		else
		{
			result = eae6320::Results::InvalidFile;
			EAE6320_ASSERTF(false, "The value must be a table");
			return result;
		}
		// finish load ActionMappings
		lua_pop(&io_luaState, 1);
	}
	// Load AxisMapping
	{
		constexpr auto* const axisMappingKey = "AxisMappings";
		lua_pushstring(&io_luaState, axisMappingKey);
		lua_gettable(&io_luaState, -2);

		if (lua_istable(&io_luaState, -1))
		{
			o_axisMappingCount = (uint8_t)luaL_len(&io_luaState, -1);

			for (uint8_t i = 1; i <= o_axisMappingCount; ++i)
			{
				lua_pushinteger(&io_luaState, i);
				lua_gettable(&io_luaState, -2);
				// Get Data here
				std::string axisName, negativeKeyName, positiveKeyName;
				{
					constexpr auto* const ActionNameKey = "AxisName";
					lua_pushstring(&io_luaState, ActionNameKey);
					lua_gettable(&io_luaState, -2);
					axisName = (std::string)lua_tostring(&io_luaState, -1);
					lua_pop(&io_luaState, 1);
				}
				{
					constexpr auto* const negativeKey_Key = "Negative_Key";
					lua_pushstring(&io_luaState, negativeKey_Key);
					lua_gettable(&io_luaState, -2);
					negativeKeyName = (std::string)lua_tostring(&io_luaState, -1);
					lua_pop(&io_luaState, 1);
				}
				{
					constexpr auto* const postiveKey_Key = "Positive_Key";
					lua_pushstring(&io_luaState, postiveKey_Key);
					lua_gettable(&io_luaState, -2);
					positiveKeyName = (std::string)lua_tostring(&io_luaState, -1);
					lua_pop(&io_luaState, 1);
				}
				o_axisMappings.insert(std::pair<std::string, FAxisBrother>(axisName, FAxisBrother(negativeKeyName, positiveKeyName)));
				lua_pop(&io_luaState, 1);
			}
		}
		else
		{
			result = eae6320::Results::InvalidFile;
			EAE6320_ASSERTF(false, "The value must be a table");
			return result;
		}
		// finish load ActionMappings
		lua_pop(&io_luaState, 1);
	}

	return result;
}

bool eae6320::Assets::cInputBuilder::ConvertKeyFromStringToKeyCode(const std::string& i_inputKey, UserInput::KeyCodes::eKeyCodes& o_keyCodes)
{
	if (i_inputKey.length() <= 0) return false;

	// it is a single key
	if (i_inputKey.length() <= 1) {
		o_keyCodes = static_cast<UserInput::KeyCodes::eKeyCodes>(i_inputKey.c_str()[0]);
		return true;
	}
	else {
		if (s_keyCodeMaps.find(i_inputKey) != s_keyCodeMaps.end()) {
			// It is a value key
			o_keyCodes = s_keyCodeMaps[i_inputKey];
			return true;
		}
		else {
			return false;
		}
	}
}
