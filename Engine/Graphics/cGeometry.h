#pragma once
#include "Engine/Assets/ReferenceCountedAssets.h"
#include <Engine/Assets/cHandle.h>
#include <Engine/Assets/cManager.h>

#if defined( EAE6320_PLATFORM_D3D )
#include "cVertexFormat.h"
struct ID3D11Buffer;
struct ID3D11DeviceContext;
//struct ID3D11RenderTargetView;
//struct ID3D11DepthStencilView;
#endif
typedef unsigned short uint16_t;
typedef unsigned int GLuint;
namespace eae6320 {
	class cResult;
	namespace Concurrency {
		class cEvent;
	}
	namespace Graphics {
		struct sInitializationParameters;
		class cConstantBuffer;
		namespace VertexFormats {
			struct s3dObject;
		}
		class cGeometry {

		public:
			// Assets
			//-------
			using Handle = Assets::cHandle<cGeometry>;
			static Assets::cManager<cGeometry> s_manager;
			static cResult Load(const std::string i_path, cGeometry*& o_ptr);
			// Initialization
			//--------------------------
			static cResult CreateGeomrtry(cGeometry*& ptr, VertexFormats::s3dObject _v_data[], uint16_t _v_length, uint16_t _idx_data[], uint16_t _idx_length);
			static void RemoveGeometry(cGeometry*& ptr);
			cResult InitializeGeometry();
			void DrawGeometry();
			cResult CleanUp();
#pragma region ReferenceCounting
				EAE6320_ASSETS_DECLAREREFERENCECOUNTINGFUNCTIONS()
				EAE6320_ASSETS_DECLAREDELETEDREFERENCECOUNTEDFUNCTIONS(cGeometry)
				EAE6320_ASSETS_DECLAREREFERENCECOUNT()
#pragma endregion


		private:
			cGeometry() 
			{
				m_VertexData = nullptr;
				m_IndexData = nullptr;
				m_indexCount = 0;
				m_vertexCount = 0;
			}
			~cGeometry();
			cGeometry(VertexFormats::s3dObject _v_data[], int _v_length, uint16_t _idx_data[], int _idx_length)
			{
				m_vertexCount = _v_length;
				m_VertexData = _v_data;
				m_indexCount = _idx_length;
				m_IndexData = _idx_data;

				//ReadOutsideData(_v_data, _v_length, _idx_data, _idx_length);
			}

			int m_vertexCount;
			int m_indexCount; 
			VertexFormats::s3dObject* m_VertexData;
			uint16_t* m_IndexData;


#if defined( EAE6320_PLATFORM_GL )
			GLuint m_vertexArrayId;
			GLuint m_vertexBufferId;
			GLuint m_IndexBufferId;

#endif

#if defined( EAE6320_PLATFORM_D3D )
			//cResult InitializeViews(const unsigned int i_resolutionWidth, const unsigned int i_resolutionHeight);
			//// In Direct3D "views" are objects that allow a texture to be used a particular way:
			//// A render target view allows a texture to have color rendered to it
			//ID3D11RenderTargetView* s_renderTargetView = nullptr;
			//// A depth/stencil view allows a texture to have depth rendered to it
			//ID3D11DepthStencilView* s_depthStencilView = nullptr;

			cVertexFormat::Handle m_vertexFormat;

			ID3D11Buffer* m_vertexBuffer;
			ID3D11Buffer* m_IndexBuffer;
#endif
		};
	}

}
