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
#include "Engine/Graphics/Direct3D/Includes.h"


eae6320::cResult eae6320::Graphics::cGeometry::InitializeGeometry()
{
	auto result = eae6320::Results::Success;
	auto* const direct3dDevice = eae6320::Graphics::sContext::g_context.direct3dDevice;
	EAE6320_ASSERT(direct3dDevice);

	// Vertex Format
	{
		if (!(result = eae6320::Graphics::cVertexFormat::s_manager.Load(eae6320::Graphics::VertexTypes::_3dObject, m_vertexFormat,
			"data/Shaders/Vertex/vertexInputLayout_3dObject.shader")))
		{
			EAE6320_ASSERTF(false, "Can't initialize geometry without vertex format");
			return result;
		}
	}
	// Vertex Buffer
	{
		const auto vertexCount = m_vertexCount;
		eae6320::Graphics::VertexFormats::s3dObject* vertexData = m_VertexData;

		D3D11_BUFFER_DESC bufferDescription{};
		{
			const auto bufferSize = vertexCount * sizeof(eae6320::Graphics::VertexFormats::s3dObject);
			EAE6320_ASSERT(bufferSize < (uint64_t(1u) << (sizeof(bufferDescription.ByteWidth) * 8)));
			bufferDescription.ByteWidth = static_cast<unsigned int>(bufferSize);
			bufferDescription.Usage = D3D11_USAGE_IMMUTABLE;	// In our class the buffer will never change after it's been created
			bufferDescription.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			bufferDescription.CPUAccessFlags = 0;	// No CPU access is necessary
			bufferDescription.MiscFlags = 0;
			bufferDescription.StructureByteStride = 0;	// Not used
		}
		D3D11_SUBRESOURCE_DATA initialData{};
		{
			initialData.pSysMem = vertexData;
			// (The other data members are ignored for non-texture buffers)
		}

		const auto d3dResult = direct3dDevice->CreateBuffer(&bufferDescription, &initialData, &m_vertexBuffer);
		if (FAILED(d3dResult))
		{
			result = eae6320::Results::Failure;
			EAE6320_ASSERTF(false, "3D object vertex buffer creation failed (HRESULT %#010x)", d3dResult);
			eae6320::Logging::OutputError("Direct3D failed to create a 3D object vertex buffer (HRESULT %#010x)", d3dResult);
			return result;
		}
	}
	// Index Buffer
	{
		const auto indexCount = m_indexCount;

		D3D11_BUFFER_DESC idxBufferDescription{};
		{
			const auto bufferSize = indexCount * sizeof(uint16_t);
			EAE6320_ASSERT(bufferSize < (uint64_t(1u) << (sizeof(idxBufferDescription.ByteWidth) * 8)));
			idxBufferDescription.ByteWidth = static_cast<unsigned int>(bufferSize);
			idxBufferDescription.Usage = D3D11_USAGE_IMMUTABLE;	// In our class the buffer will never change after it's been created
			idxBufferDescription.BindFlags = D3D11_BIND_INDEX_BUFFER;
			idxBufferDescription.CPUAccessFlags = 0;	// No CPU access is necessary
			idxBufferDescription.MiscFlags = 0;
			idxBufferDescription.StructureByteStride = 0;	// Not used
		}
		D3D11_SUBRESOURCE_DATA idxInitialData{};
		{
			idxInitialData.pSysMem = m_IndexData;
			// (The other data members are ignored for non-texture buffers)
		}

		const auto d3dResult = direct3dDevice->CreateBuffer(&idxBufferDescription, &idxInitialData, &m_IndexBuffer);
		if (FAILED(d3dResult))
		{
			result = eae6320::Results::Failure;
			EAE6320_ASSERTF(false, "3D object index buffer creation failed (HRESULT %#010x)", d3dResult);
			eae6320::Logging::OutputError("Direct3D failed to create a 3D object index buffer (HRESULT %#010x)", d3dResult);
			return result;
		}
	}
	return result;
}

void eae6320::Graphics::cGeometry::DrawGeometry()
{

	auto* const direct3dImmediateContext = sContext::g_context.direct3dImmediateContext;
	EAE6320_ASSERT(direct3dImmediateContext);

	// Bind a specific vertex buffer to the device as a data source
	{
		EAE6320_ASSERT(m_vertexBuffer);
		constexpr unsigned int startingSlot = 0;
		constexpr unsigned int vertexBufferCount = 1;
		// The "stride" defines how large a single vertex is in the stream of data
		constexpr unsigned int bufferStride = sizeof(VertexFormats::s3dObject);
		// It's possible to start streaming data in the middle of a vertex buffer
		constexpr unsigned int bufferOffset = 0;
		direct3dImmediateContext->IASetVertexBuffers(startingSlot, vertexBufferCount, &m_vertexBuffer, &bufferStride, &bufferOffset);
	}
	// bind the index buffer
	{
		EAE6320_ASSERT(m_IndexBuffer);
		constexpr DXGI_FORMAT indexFormat = DXGI_FORMAT_R16_UINT;
		// The indices start at the beginning of the buffer
		constexpr unsigned int offset = 0;
		direct3dImmediateContext->IASetIndexBuffer(m_IndexBuffer, indexFormat, offset);
	}
	// Specify what kind of data the vertex buffer holds
	{
		// Bind the vertex format (which defines how to interpret a single vertex)
		{
			EAE6320_ASSERT(m_vertexFormat);
			auto* const vertexFormat = cVertexFormat::s_manager.Get(m_vertexFormat);
			EAE6320_ASSERT(vertexFormat);
			vertexFormat->Bind();
		}
		// Set the topology (which defines how to interpret multiple vertices as a single "primitive";
		// the vertex buffer was defined as a triangle list
		// (meaning that every primitive is a triangle and will be defined by three vertices)
		direct3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}
	// Render triangles from the currently-bound vertex buffer
	{
		// As of this comment only a single triangle is drawn
		// (you will have to update this code in future assignments!)

		//unsigned int triangleCount = triangleCount;
		//auto vertexCountToRender = m_vertexCount;
		//// It's possible to start rendering primitives in the middle of the stream
		//constexpr unsigned int indexOfFirstVertexToRender = 0;
		//m_direct3dImmediateContext->Draw(vertexCountToRender, indexOfFirstVertexToRender);


		constexpr unsigned int indexOfFirstIndexToUse = 0;
		constexpr unsigned int offsetToAddToEachIndex = 0;
		direct3dImmediateContext->DrawIndexed(static_cast<unsigned int>(m_indexCount), indexOfFirstIndexToUse, offsetToAddToEachIndex);
	}
}
eae6320::cResult eae6320::Graphics::cGeometry::CleanUp()
{
	auto result = Results::Success;

	if (m_vertexBuffer)
	{
		(m_vertexBuffer)->Release();
		m_vertexBuffer = nullptr;
	}
	if (m_IndexBuffer) {
		(m_IndexBuffer)->Release();
		m_IndexBuffer = nullptr;
	}
	if (m_vertexFormat)
	{
		const auto result_vertexFormat = cVertexFormat::s_manager.Release(m_vertexFormat);
		if (!result_vertexFormat)
		{
			EAE6320_ASSERT(false);
			if (result)
			{
				result = result_vertexFormat;
			}
		}
	}

	return result;
}
