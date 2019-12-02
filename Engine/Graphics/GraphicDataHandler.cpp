#include "GraphicDataHandler.h"
#include "Engine/Results/cResult.h"
#include "Engine/Asserts/Asserts.h"
#include "Engine/Results/Results.h"
#include "sContext.h"
#include "cRenderState.h"
#include "cShader.h"
#include "cConstantBuffer.h"
#include "Engine/Concurrency/cEvent.h"
#include "cGeometry.h"
#include "cEffect.h"
#include <map>
#include <vector>

#if defined( EAE6320_PLATFORM_D3D )
#include "Engine/Graphics/cVertexFormat.h"
#include "Engine/Graphics/Direct3D/Includes.h"
#include <Engine/ScopeGuard/cScopeGuard.h>
#endif

#if defined( EAE6320_PLATFORM_D3D )
// In Direct3D "views" are objects that allow a texture to be used a particular way:
// A render target view allows a texture to have color rendered to it
ID3D11RenderTargetView* s_renderTargetView = nullptr;
// A depth/stencil view allows a texture to have depth rendered to it
ID3D11DepthStencilView* s_depthStencilView = nullptr;
#endif

float s_clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
eae6320::Graphics::cRenderState::Handle eae6320::Graphics::DataHandler::s_renderState;
eae6320::cResult eae6320::Graphics::DataHandler::InitializeGlobalData(const sInitializationParameters& i_initializationParameters, cConstantBuffer& s_constantBuffer_frame, cConstantBuffer& s_constantBuffer_drawCall, Concurrency::cEvent& s_whenAllDataHasBeenSubmittedFromApplicationThread, Concurrency::cEvent& s_whenDataForANewFrameCanBeSubmittedFromApplicationThread)
{
	auto result = Results::Success;

	// Initialize the platform-specific context
	if (!(result = sContext::g_context.Initialize(i_initializationParameters)))
	{
		EAE6320_ASSERTF(false, "Can't initialize Graphics without context");
		return result;
	}

	// Initialize the asset managers
	{
		if (!(result = cRenderState::s_manager.Initialize()))
		{
			EAE6320_ASSERTF(false, "Can't initialize Graphics without the render state manager");
			return result;
		}
		if (!(result = cShader::s_manager.Initialize()))
		{
			EAE6320_ASSERTF(false, "Can't initialize Graphics without the shader manager");
			return result;
		}
#if defined( EAE6320_PLATFORM_D3D )
		if (!(result = cVertexFormat::s_manager.Initialize()))
		{
			EAE6320_ASSERTF(false, "Can't initialize Graphics without the vertex format manager");
			return result;
		}
#endif

	}

	// Initialize the platform-independent graphics objects
	{
		if (result = s_constantBuffer_frame.Initialize())
		{
			// There is only a single frame constant buffer that is reused
			// and so it can be bound at initialization time and never unbound
			s_constantBuffer_frame.Bind(
				// In our class both vertex and fragment shaders use per-frame constant data
				ShaderTypes::Vertex | ShaderTypes::Fragment);
		}
		else
		{
			EAE6320_ASSERTF(false, "Can't initialize Graphics without frame constant buffer");
			return result;
		}
	}
	{
		if (result = s_constantBuffer_drawCall.Initialize())
		{
			// There is only a single drawCall constant buffer that is reused
			// and so it can be bound at initialization time and never unbound
			s_constantBuffer_drawCall.Bind(
				// In our class both vertex and fragment shaders use per-frame constant data
				ShaderTypes::Vertex | ShaderTypes::Fragment);
		}
		else
		{
			EAE6320_ASSERTF(false, "Can't initialize Graphics without drawcall constant buffer");
			return result;
		}
	}
	// Initialize the events
	{
		if (!(result = s_whenAllDataHasBeenSubmittedFromApplicationThread.Initialize(Concurrency::EventType::ResetAutomaticallyAfterBeingSignaled)))
		{
			EAE6320_ASSERTF(false, "Can't initialize Graphics without event for when data has been submitted from the application thread");
			return result;
		}
		if (!(result = s_whenDataForANewFrameCanBeSubmittedFromApplicationThread.Initialize(Concurrency::EventType::ResetAutomaticallyAfterBeingSignaled,
			Concurrency::EventState::Signaled)))
		{
			EAE6320_ASSERTF(false, "Can't initialize Graphics without event for when data can be submitted from the application thread");
			return result;
		}
	}
#if defined( EAE6320_PLATFORM_D3D )
	// Initialize the views
	{
		if (!(result = InitializeViews(i_initializationParameters.resolutionWidth, i_initializationParameters.resolutionHeight)))
		{
			EAE6320_ASSERTF(false, "Can't initialize Graphics without the views");
			return result;
		}
	}
#endif

	return result;
}

eae6320::cResult eae6320::Graphics::DataHandler::CleanUpGlobalData()
{
	auto result = Results::Success;
	if (s_renderState)
	{
		const auto result_renderState = cRenderState::s_manager.Release(s_renderState);
		if (!result_renderState)
		{
			EAE6320_ASSERT(false);
			if (result)
			{
				result = result_renderState;
			}
		}
	}
	{
		const auto result_shaderManager = cShader::s_manager.CleanUp();
		if (!result_shaderManager)
		{
			EAE6320_ASSERT(false);
			if (result)
			{
				result = result_shaderManager;
			}
		}
	}
	{
		const auto result_renderStateManager = cRenderState::s_manager.CleanUp();
		if (!result_renderStateManager)
		{
			EAE6320_ASSERT(false);
			if (result)
			{
				result = result_renderStateManager;
			}
		}
	}
#if defined( EAE6320_PLATFORM_D3D )
	{
		if (s_renderTargetView)
		{
			s_renderTargetView->Release();
			s_renderTargetView = nullptr;
		}
		if (s_depthStencilView)
		{
			s_depthStencilView->Release();
			s_depthStencilView = nullptr;
		}

		const auto result_vertexFormatManager = cVertexFormat::s_manager.CleanUp();
		if (!result_vertexFormatManager)
		{
			EAE6320_ASSERT(false);
			if (result)
			{
				result = result_vertexFormatManager;
			}
		}
	}
#endif
	
	{
		const auto result_context = sContext::g_context.CleanUp();
		if (!result_context)
		{
			EAE6320_ASSERT(false);
			if (result)
			{
				result = result_context;
			}
		}
	}
	return result;
}

void eae6320::Graphics::DataHandler::SetClearColor(float _r, float _g, float _b, float _a)
{
	s_clearColor[0] = _r;
	s_clearColor[1] = _g;
	s_clearColor[2] = _b;
	s_clearColor[3] = _a;
}

void eae6320::Graphics::DataHandler::ClearColor()
{
#if defined( EAE6320_PLATFORM_D3D )

	auto* const direct3dImmediateContext = sContext::g_context.direct3dImmediateContext;
	EAE6320_ASSERT(direct3dImmediateContext);


	{
		EAE6320_ASSERT(s_renderTargetView);

		// Black is usually used
		direct3dImmediateContext->ClearRenderTargetView(s_renderTargetView, s_clearColor);
	}
	// In addition to the color buffer there is also a hidden image called the "depth buffer"
	// which is used to make it less important which order draw calls are made.
	// It must also be "cleared" every frame just like the visible color buffer.
	{
		EAE6320_ASSERT(s_depthStencilView);

		constexpr float clearToFarDepth = 1.0f;
		constexpr uint8_t stencilValue = 0;	// Arbitrary if stencil isn't used
		direct3dImmediateContext->ClearDepthStencilView(s_depthStencilView, D3D11_CLEAR_DEPTH, clearToFarDepth, stencilValue);
	}
#elif defined( EAE6320_PLATFORM_GL )

	{
		// Black is usually used
		{
			glClearColor(s_clearColor[0], s_clearColor[1], s_clearColor[2], s_clearColor[3]);
			EAE6320_ASSERT(glGetError() == GL_NO_ERROR);
		}
		{
			constexpr GLbitfield clearColor = GL_COLOR_BUFFER_BIT;
			glClear(clearColor);
			EAE6320_ASSERT(glGetError() == GL_NO_ERROR);
		}
	}
	// In addition to the color buffer there is also a hidden image called the "depth buffer"
	// which is used to make it less important which order draw calls are made.
	// It must also be "cleared" every frame just like the visible color buffer.
	{
		{
			glDepthMask(GL_TRUE);
			EAE6320_ASSERT(glGetError() == GL_NO_ERROR);
			constexpr GLclampd clearToFarDepth = 1.0;
			glClearDepth(clearToFarDepth);
			EAE6320_ASSERT(glGetError() == GL_NO_ERROR);
		}
		{
			constexpr GLbitfield clearDepth = GL_DEPTH_BUFFER_BIT;
			glClear(clearDepth);
			EAE6320_ASSERT(glGetError() == GL_NO_ERROR);
		}
	}
#endif
}

void eae6320::Graphics::DataHandler::SwapBuffer()
{
#if defined( EAE6320_PLATFORM_GL )
	// Everything has been drawn to the "back buffer", which is just an image in memory.
// In order to display it the contents of the back buffer must be "presented"
// (or "swapped" with the "front buffer", which is the image that is actually being displayed)
	{
		const auto deviceContext = sContext::g_context.deviceContext;
		EAE6320_ASSERT(deviceContext != NULL);
		const auto glResult = SwapBuffers(deviceContext);
		EAE6320_ASSERT(glResult != FALSE);
	}
#endif
#if defined( EAE6320_PLATFORM_D3D )
	// Everything has been drawn to the "back buffer", which is just an image in memory.
// In order to display it the contents of the back buffer must be "presented"
// (or "swapped" with the "front buffer", which is the image that is actually being displayed)
	{
		auto* const swapChain = sContext::g_context.swapChain;
		EAE6320_ASSERT(swapChain);
		constexpr unsigned int swapImmediately = 0;
		constexpr unsigned int presentNextFrame = 0;
		const auto result = swapChain->Present(swapImmediately, presentNextFrame);
		EAE6320_ASSERT(SUCCEEDED(result));
	}
#endif
}

#if defined( EAE6320_PLATFORM_D3D )
eae6320::cResult eae6320::Graphics::DataHandler::InitializeViews(const unsigned int i_resolutionWidth, const unsigned int i_resolutionHeight)
{
	auto result = eae6320::Results::Success;

	ID3D11Texture2D* backBuffer = nullptr;
	ID3D11Texture2D* depthBuffer = nullptr;
	eae6320::cScopeGuard scopeGuard([&backBuffer, &depthBuffer]
		{
			// Regardless of success or failure the two texture resources should be released
			// (if the function is successful the views will hold internal references to the resources)
			if (backBuffer)
			{
				backBuffer->Release();
				backBuffer = nullptr;
			}
			if (depthBuffer)
			{
				depthBuffer->Release();
				depthBuffer = nullptr;
			}
		});

	auto& g_context = eae6320::Graphics::sContext::g_context;
	auto* const direct3dDevice = g_context.direct3dDevice;
	EAE6320_ASSERT(direct3dDevice);
	auto* const direct3dImmediateContext = g_context.direct3dImmediateContext;
	EAE6320_ASSERT(direct3dImmediateContext);

	// Create a "render target view" of the back buffer
	// (the back buffer was already created by the call to D3D11CreateDeviceAndSwapChain(),
	// but a "view" of it is required to use as a "render target",
	// meaning a texture that the GPU can render to)
	{
		// Get the back buffer from the swap chain
		{
			constexpr unsigned int bufferIndex = 0;	// This must be 0 since the swap chain is discarded
			const auto d3dResult = g_context.swapChain->GetBuffer(bufferIndex, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer));
			if (FAILED(d3dResult))
			{
				result = eae6320::Results::Failure;
				EAE6320_ASSERTF(false, "Couldn't get the back buffer from the swap chain (HRESULT %#010x)", d3dResult);
				eae6320::Logging::OutputError("Direct3D failed to get the back buffer from the swap chain (HRESULT %#010x)", d3dResult);
				return result;
			}
		}
		// Create the view
		{
			constexpr D3D11_RENDER_TARGET_VIEW_DESC* const accessAllSubResources = nullptr;
			const auto d3dResult = direct3dDevice->CreateRenderTargetView(backBuffer, accessAllSubResources, &s_renderTargetView);
			if (FAILED(d3dResult))
			{
				result = eae6320::Results::Failure;
				EAE6320_ASSERTF(false, "Couldn't create render target view (HRESULT %#010x)", d3dResult);
				eae6320::Logging::OutputError("Direct3D failed to create the render target view (HRESULT %#010x)", d3dResult);
				return result;
			}
		}
	}
	// Create a depth/stencil buffer and a view of it
	{
		// Unlike the back buffer no depth/stencil buffer exists until and unless it is explicitly created
		{
			D3D11_TEXTURE2D_DESC textureDescription{};
			{
				textureDescription.Width = i_resolutionWidth;
				textureDescription.Height = i_resolutionHeight;
				textureDescription.MipLevels = 1;	// A depth buffer has no MIP maps
				textureDescription.ArraySize = 1;
				textureDescription.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;	// 24 bits for depth and 8 bits for stencil
				{
					DXGI_SAMPLE_DESC& sampleDescription = textureDescription.SampleDesc;

					sampleDescription.Count = 1;	// No multisampling
					sampleDescription.Quality = 0;	// Doesn't matter when Count is 1
				}
				textureDescription.Usage = D3D11_USAGE_DEFAULT;	// Allows the GPU to write to it
				textureDescription.BindFlags = D3D11_BIND_DEPTH_STENCIL;
				textureDescription.CPUAccessFlags = 0;	// CPU doesn't need access
				textureDescription.MiscFlags = 0;
			}
			// The GPU renders to the depth/stencil buffer and so there is no initial data
			// (like there would be with a traditional texture loaded from disk)
			constexpr D3D11_SUBRESOURCE_DATA* const noInitialData = nullptr;
			const auto d3dResult = direct3dDevice->CreateTexture2D(&textureDescription, noInitialData, &depthBuffer);
			if (FAILED(d3dResult))
			{
				result = eae6320::Results::Failure;
				EAE6320_ASSERTF(false, "Couldn't create depth buffer (HRESULT %#010x)", d3dResult);
				eae6320::Logging::OutputError("Direct3D failed to create the depth buffer resource (HRESULT %#010x)", d3dResult);
				return result;
			}
		}
		// Create the view
		{
			constexpr D3D11_DEPTH_STENCIL_VIEW_DESC* const noSubResources = nullptr;
			const auto d3dResult = direct3dDevice->CreateDepthStencilView(depthBuffer, noSubResources, &s_depthStencilView);
			if (FAILED(d3dResult))
			{
				result = eae6320::Results::Failure;
				EAE6320_ASSERTF(false, "Couldn't create depth stencil view (HRESULT %#010x)", d3dResult);
				eae6320::Logging::OutputError("Direct3D failed to create the depth stencil view (HRESULT %#010x)", d3dResult);
				return result;
			}
		}
	}

	// Bind the views
	{
		constexpr unsigned int renderTargetCount = 1;
		direct3dImmediateContext->OMSetRenderTargets(renderTargetCount, &s_renderTargetView, s_depthStencilView);
	}
	// Specify that the entire render target should be visible
	{
		D3D11_VIEWPORT viewPort{};
		{
			viewPort.TopLeftX = viewPort.TopLeftY = 0.0f;
			viewPort.Width = static_cast<float>(i_resolutionWidth);
			viewPort.Height = static_cast<float>(i_resolutionHeight);
			viewPort.MinDepth = 0.0f;
			viewPort.MaxDepth = 1.0f;
		}
		constexpr unsigned int viewPortCount = 1;
		direct3dImmediateContext->RSSetViewports(viewPortCount, &viewPort);
	}

	return result;
}
#endif
