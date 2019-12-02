/*
	This class builds inputs
*/

#ifndef EAE6320_CSHADERBUILDER_H
#define EAE6320_CSHADERBUILDER_H

// Includes
//=========

#include <Tools/AssetBuildLibrary/cbBuilder.h>

#include <Engine/Graphics/Configuration.h>
#include "Engine/UserInput/UserInput.h"
#include <map>
// Class Declaration
//==================
struct FAxisBrother {
	FAxisBrother(std::string i_nKey, std::string i_pKey) : negative_key(i_nKey), positive_key(i_pKey) {}
	std::string negative_key;
	std::string positive_key;
};
namespace eae6320
{
	namespace Assets
	{

		class cInputBuilder : public cbBuilder
		{

			// Inherited Implementation
			//=========================
		private:

			// Build
			//------

			virtual cResult Build(const std::vector<std::string>& i_arguments) override;
			// Load from lua
			//------
			virtual cResult Load(const std::string i_path, std::map<std::string, std::string>& o_actionMappings, uint8_t& o_actionMappingCount, std::map<std::string, FAxisBrother>& o_axisMappings, uint8_t& o_axisMappingCount);

			bool ConvertKeyFromStringToKeyCode(const std::string& i_inputKey, UserInput::KeyCodes::eKeyCodes& o_keyCodes);
			// Implementation
			//===============
		};
	}
}

#endif	// EAE6320_CSHADERBUILDER_H
