#include "cGeometry.h"
#include "VertexFormats.h"
#include "Engine/Math/sVector.h"
#include "Engine/Results/cResult.h"
#include "Engine/Results/Results.h"
#include "sContext.h"
#include "Engine/Asserts/Asserts.h"
#include "cRenderState.h"
#include "cShader.h"
#include <fstream>
#include <Engine/ScopeGuard/cScopeGuard.h>
#include "Engine/Platform/Platform.h"


eae6320::Assets::cManager<eae6320::Graphics::cGeometry> eae6320::Graphics::cGeometry::s_manager;

eae6320::Graphics::cGeometry::~cGeometry()
{
	auto result = Results::Success;
	result = CleanUp();
	EAE6320_ASSERT(result);
}

eae6320::cResult eae6320::Graphics::cGeometry::Load(const std::string i_path, cGeometry*& o_ptr)
{
	auto result = Results::Success;

		eae6320::Platform::sDataFromFile inFile;
		if (!(result = eae6320::Platform::LoadBinaryFile(i_path.c_str(), inFile))) {
			return result;
		}
		
		auto currentOffset = reinterpret_cast<uintptr_t>(inFile.data);
		//const auto finalOffset = currentOffset + size;

		const auto vertexCount = *reinterpret_cast<uint16_t*>(currentOffset);
		currentOffset += sizeof(uint16_t);

		auto* const vertexData = reinterpret_cast<VertexFormats::s3dObject*>(currentOffset);
		currentOffset += sizeof(VertexFormats::s3dObject) * vertexCount;

		const auto indexCount = *reinterpret_cast<uint16_t*>(currentOffset);
		currentOffset += sizeof(indexCount);

		auto* const indexData = reinterpret_cast<uint16_t*>(currentOffset);
		currentOffset += sizeof(indexData);

		result = CreateGeomrtry(o_ptr, vertexData, vertexCount, indexData, indexCount);

	return result;
}

eae6320::cResult eae6320::Graphics::cGeometry::CreateGeomrtry(cGeometry*& ptr, VertexFormats::s3dObject _v_data[], uint16_t _v_length, uint16_t _idx_data[], uint16_t _idx_length)
{
	auto result = Results::Success;
	ptr = new cGeometry(_v_data, _v_length, _idx_data, _idx_length);
	result = ptr->InitializeGeometry();
	return result;
}

void eae6320::Graphics::cGeometry::RemoveGeometry(cGeometry*& ptr)
{
	if (ptr)
		ptr->DecrementReferenceCount();
	ptr = nullptr;
}




