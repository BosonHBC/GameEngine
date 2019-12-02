#include <string>
#include "cEffect.h"
#include "Engine/Results/Results.h"
#include "Engine/ScopeGuard/cScopeGuard.h"
#include "GraphicDataHandler.h"
#include "cShader.h"
#include "Engine/Graphics/Direct3D/Includes.h"
#include "sContext.h"

namespace eae6320 {
	namespace Graphics {

		eae6320::cResult cEffect::InitializeShadingData(const char* i_vertexShaderPath, const char* i_fragmentShaderPath, const uint8_t& i_renderState)
		{
			auto result = Results::Success;
			if (!DataHandler::s_renderState.IsValid())
				if (!(result = cRenderState::s_manager.Load(i_renderState, DataHandler::s_renderState)))
				{
					EAE6320_ASSERTF(false, "Can't initialize shading data without render state");
				}

			if (!(result = cShader::s_manager.Load(i_vertexShaderPath,
				s_vertexShader, ShaderTypes::Vertex)))
			{
				EAE6320_ASSERTF(false, "Can't initialize shading data without vertex shader");
			}
			if (!(result = cShader::s_manager.Load(i_fragmentShaderPath,
				s_fragmentShader, ShaderTypes::Fragment)))
			{
				EAE6320_ASSERTF(false, "Can't initialize shading data without fragment shader");
			}

			return result;
		}
		void cEffect::BindShadingData()
		{

			auto* const direct3dImmediateContext = sContext::g_context.direct3dImmediateContext;
			EAE6320_ASSERT(direct3dImmediateContext);

			constexpr ID3D11ClassInstance* const* noInterfaces = nullptr;
			constexpr unsigned int interfaceCount = 0;
			// Vertex shader
			{
				EAE6320_ASSERT(s_vertexShader);
				auto* const shader = cShader::s_manager.Get(s_vertexShader);
				EAE6320_ASSERT(shader && shader->m_shaderObject.vertex);
				direct3dImmediateContext->VSSetShader(shader->m_shaderObject.vertex, noInterfaces, interfaceCount);
			}
			// Fragment shader
			{
				EAE6320_ASSERT(s_fragmentShader);
				auto* const shader = cShader::s_manager.Get(s_fragmentShader);
				EAE6320_ASSERT(shader && shader->m_shaderObject.fragment);
				direct3dImmediateContext->PSSetShader(shader->m_shaderObject.fragment, noInterfaces, interfaceCount);
			}
			// Render state
			{
				EAE6320_ASSERT(eae6320::Graphics::DataHandler::s_renderState);
				auto* const renderState = cRenderState::s_manager.Get(eae6320::Graphics::DataHandler::s_renderState);
				EAE6320_ASSERT(renderState);
				renderState->Bind();
			}

		}
		eae6320::cResult cEffect::CleanUp()
		{
			auto result = Results::Success;

			if (s_vertexShader)
			{
				const auto result_vertexShader = cShader::s_manager.Release(s_vertexShader);
				if (!result_vertexShader)
				{
					EAE6320_ASSERT(false);
					if (result)
					{
						result = result_vertexShader;
					}
				}
			}
			if (s_fragmentShader)
			{
				const auto result_fragmentShader = cShader::s_manager.Release(s_fragmentShader);
				if (!result_fragmentShader)
				{
					EAE6320_ASSERT(false);
					if (result)
					{
						result = result_fragmentShader;
					}
				}
			}


			return result;
		}
	}
}