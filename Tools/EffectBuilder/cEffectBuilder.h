/*
	This class builds shaders
*/

#ifndef EAE6320_CSHADERBUILDER_H
#define EAE6320_CSHADERBUILDER_H

// Includes
//=========

#include <Tools/AssetBuildLibrary/cbBuilder.h>

#include <Engine/Graphics/Configuration.h>
#include <Engine/Graphics/cEffect.h>

// Class Declaration
//==================

namespace eae6320
{
	namespace Assets
	{
		class cEffectBuilder : public cbBuilder
		{
			// Inherited Implementation
			//=========================

		private:

			// Build
			//------

			virtual cResult Build(const std::vector<std::string>& i_arguments) override;

			// Load from lua
			//------
			virtual cResult Load(const std::string i_path, std::string& o_vertexShaderPath, std::string& o_fragmentShaderPath, uint8_t*& o_renderState);

			// Implementation
			//===============
		};
	}
}

#endif	// EAE6320_CSHADERBUILDER_H
