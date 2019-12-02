#include <string>
#include "cEffect.h"
#include "Engine/Results/Results.h"
#include "Engine/ScopeGuard/cScopeGuard.h"
#include "GraphicDataHandler.h"
#include "cShader.h"
#include "Engine/Platform/Platform.h"

eae6320::Assets::cManager<eae6320::Graphics::cEffect> eae6320::Graphics::cEffect::s_manager;

namespace eae6320 {
	namespace Graphics {


		eae6320::cResult cEffect::Load(const std::string i_path, cEffect*& o_ptr)
		{
			auto result = Results::Success;

			eae6320::Platform::sDataFromFile inFile;
			if (!(result = eae6320::Platform::LoadBinaryFile(i_path.c_str(), inFile))) {
				return result;
			}

			auto currentOffset = reinterpret_cast<uintptr_t>(inFile.data);
			const auto renderState = *reinterpret_cast<uint8_t*>(currentOffset);
			currentOffset += sizeof(uint8_t);
			const auto vertexShaderPath = reinterpret_cast<const char*>(currentOffset);
			// offset += length of vertex shader path + null terminal
			currentOffset += strlen(vertexShaderPath) + 1;
			const auto fragmentShaderPath = reinterpret_cast<const char*>(currentOffset);
			//currentOffset += strlen(fragmentShaderPath) + 1;
			result = CreateEffect(o_ptr, vertexShaderPath, fragmentShaderPath, renderState);

			return result;
		}

		eae6320::cResult cEffect::CreateEffect(cEffect*& ptr, const char* i_vertexShaderPath, const char* i_fragmentShaderPath, const uint8_t& i_renderState)
		{
			auto result = Results::Success;
			ptr = new cEffect();
			result = ptr->InitializeShadingData(i_vertexShaderPath, i_fragmentShaderPath, i_renderState);
			
			return result;
		}

		void cEffect::RemoveEffect(cEffect*& ptr)
		{
			if (ptr)
				ptr->DecrementReferenceCount();
			ptr = nullptr;
		}


		cEffect::~cEffect()
		{
			auto result = Results::Success;
			result = CleanUp();
			EAE6320_ASSERT(result);
		}

	}
}





