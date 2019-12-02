/*
	This file defines the layout of the geometric data
	that the CPU sends to the GPU

	These must exactly match the data passed in to vertex shader programs.
*/

#ifndef EAE6320_GRAPHICS_VERTEXBUFFERFORMATS_H
#define EAE6320_GRAPHICS_VERTEXBUFFERFORMATS_H

// Includes
//=========

#include "Configuration.h"

// Format Definitions
//===================

namespace eae6320
{
	namespace Graphics
	{
		namespace VertexFormats
		{
			// In our class we will only have a single vertex format for all 3D objects.
			// In a real game it would be more common to have several different formats
			// (with simpler/smaller formats for simpler shading
			// and more complex and bigger formats for more complicated shading).
			struct s3dObject
			{
				// POSITION
				// 3 floats == 12 bytes
				// Offset = 0
				float x, y, z;
			};
		}
	}
}

#endif	// EAE6320_GRAPHICS_VERTEXBUFFERFORMATS_H
