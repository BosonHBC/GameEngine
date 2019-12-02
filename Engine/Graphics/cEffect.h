#pragma once
#include "Engine/Assets/ReferenceCountedAssets.h"
#include "cShader.h"
#if defined( EAE6320_PLATFORM_D3D )
struct ID3D11DeviceContext;
#endif
namespace eae6320 {
	class cResult;
	namespace Graphics {

		class cEffect {

		public:

			// Assets
			//-------
			using Handle = Assets::cHandle<cEffect>;
			static Assets::cManager<cEffect> s_manager;
			static cResult Load(const std::string i_path, cEffect*& o_ptr);
			// Initialization
			//--------------------------
			static cResult CreateEffect(cEffect*& ptr, const char* i_vertexShaderPath, const char* i_fragmentShaderPath, const uint8_t& i_renderState);
			static void RemoveEffect(cEffect*& ptr);
			cResult InitializeShadingData(const char* i_vertexShaderPath, const char* i_fragmentShaderPath, const uint8_t& i_renderState);
			void BindShadingData();
			cResult CleanUp();

#pragma region ReferenceCounting
			EAE6320_ASSETS_DECLAREREFERENCECOUNTINGFUNCTIONS()
				EAE6320_ASSETS_DECLAREDELETEDREFERENCECOUNTEDFUNCTIONS(cEffect)
				EAE6320_ASSETS_DECLAREREFERENCECOUNT()
#pragma endregion

#if defined (EAE6320_PLATFORM_GL)
			unsigned int m_programID;
			cResult CreateProgram();
#endif
		private:
			cEffect() {}
			~cEffect();

			cShader::Handle s_vertexShader;
			cShader::Handle s_fragmentShader;

		};
	}
}