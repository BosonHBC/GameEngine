/*
	This class builds shaders
*/

#ifndef EAE6320_CSHADERBUILDER_H
#define EAE6320_CSHADERBUILDER_H

// Includes
//=========

#include <Tools/AssetBuildLibrary/cbBuilder.h>

#include <Engine/Graphics/Configuration.h>
#include <Engine/Graphics/cGeometry.h>

// Class Declaration
//==================

namespace eae6320
{
	namespace Assets
	{
		class cGeometryBuilder : public cbBuilder
		{
			// Inherited Implementation
			//=========================

		private:

			// Build
			//------

			virtual cResult Build(const std::vector<std::string>& i_arguments) override;

			// Load from lua
			//------
			virtual cResult Load(const std::string i_path, uint8_t& o_leftHand, uint16_t& o_vertexCount, eae6320::Graphics::VertexFormats::s3dObject*& o_vertexes, uint16_t& o_indexCount,uint16_t*& o_indices);

			void FixIndexData(uint16_t*& o_indicesData, uint16_t i_indexCount, bool i_useLeftHand);
			// Implementation
			//===============
		};
	}
}

#endif	// EAE6320_CSHADERBUILDER_H
