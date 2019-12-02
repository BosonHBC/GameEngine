#pragma once
#if defined( EAE6320_PLATFORM_D3D )
struct ID3D11RenderTargetView;
struct ID3D11DepthStencilView;
#endif
#include "cRenderState.h"

namespace eae6320
{
	class cResult;
	namespace Concurrency {
		class cEvent;
	}
	namespace Graphics
	{
		struct sInitializationParameters;
		class cConstantBuffer;
		class cEffect;
		class cGeometry;

		namespace DataHandler {

			cResult InitializeGlobalData(const  sInitializationParameters& i_initializationParameters, cConstantBuffer& s_constantBuffer_frame, cConstantBuffer& s_constantBuffer_drawCall, Concurrency::cEvent& s_whenAllDataHasBeenSubmittedFromApplicationThread, Concurrency::cEvent& s_whenDataForANewFrameCanBeSubmittedFromApplicationThread);
			cResult CleanUpGlobalData();

			void SetClearColor(float _r, float _g, float _b, float _a);
			void ClearColor();

			void SwapBuffer();

			extern cRenderState::Handle s_renderState;
#if defined( EAE6320_PLATFORM_D3D )

			cResult InitializeViews(const unsigned int i_resolutionWidth, const unsigned int i_resolutionHeight);
#endif
		}


	}
}