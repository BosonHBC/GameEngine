/*
	A vertex format determines how the GPU interprets
	the geometric data that the CPU sends it

	It is important to differentiate a "vertex format" object from the actual vertex data:
		* An instance of this cVertexFormat class is an object owned by the platform-specific graphics API
		* The vertex data is an array of one of the structs defined in VertexFormats.h,
			but in order for it to be used in a draw call
			the GPU must use a vertex format object to know how to interpret the data in the vertex shader program
*/

#ifndef EAE6320_GRAPHICS_VERTEXFORMATS_H
#define EAE6320_GRAPHICS_VERTEXFORMATS_H

// Includes
//=========

#include "Configuration.h"

#include <cstdint>
#include <Engine/Assets/cHandle.h>
#include <Engine/Assets/cManager.h>
#include <Engine/Assets/ReferenceCountedAssets.h>
#include <Engine/Results/Results.h>

// Forward Declarations
//=====================

#ifdef EAE6320_PLATFORM_D3D
	struct ID3D11InputLayout;
#endif

// Vertex Types
//=============

namespace eae6320
{
	namespace Graphics
	{
		// In our class we will define one type of vertex
		// (see VertexFormats.h for the data layout of each type)
		enum class VertexTypes : uint8_t
		{
			//	* 3D Object:
			//		* This is the vertex used by 3D geometry in world space
			//			(which is the only kind of geometry we will render in this class)
			_3dObject,
		};
	}
}

// Class Declaration
//==================

namespace eae6320
{
	namespace Graphics
	{
		class cVertexFormat
		{
			// Interface
			//==========

		public:

			// Render
			//-------

			void Bind();

			// Access
			//-------

			using Handle = Assets::cHandle<cVertexFormat>;
			static Assets::cManager<cVertexFormat, VertexTypes> s_manager;

			// Initialization / Clean Up
			//--------------------------

			static cResult Load( const VertexTypes i_type, cVertexFormat*& o_vertexFormat, const char* const i_vertexShaderPath );

			EAE6320_ASSETS_DECLAREDELETEDREFERENCECOUNTEDFUNCTIONS( cVertexFormat );

			// Reference Counting
			//-------------------

			EAE6320_ASSETS_DECLAREREFERENCECOUNTINGFUNCTIONS();

			// Data
			//=====

		private:

#if defined( EAE6320_PLATFORM_D3D )
			// D3D has an "input layout" object that associates the layout of the vertex format struct
			// with the input from a vertex shader
			ID3D11InputLayout* m_vertexInputLayout = nullptr;
#endif
			
			EAE6320_ASSETS_DECLAREREFERENCECOUNT();

			// Implementation
			//===============

		private:

			// Initialization / Clean Up
			//--------------------------

			cResult Initialize( const VertexTypes i_type, const char* const i_vertexShaderPath );
			cResult CleanUp();

			cVertexFormat() = default;
			~cVertexFormat();
		};
	}
}

#endif	// EAE6320_GRAPHICS_VERTEXFORMATS_H
