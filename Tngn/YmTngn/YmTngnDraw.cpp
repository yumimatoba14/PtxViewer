#include "pch.h"
#include "YmTngnDraw.h"
#include "YmTngnShaderImpl.h"
#include "YmDx11MappedSubResource.h"

using namespace std;
using namespace Ymcpp;

////////////////////////////////////////////////////////////////////////////////

YmTngnDraw::~YmTngnDraw()
{
}

////////////////////////////////////////////////////////////////////////////////

bool YmTngnDraw::IsProgressiveViewMode() const
{
	return m_pShaderImpl->IsProgressiveViewMode();
}

bool YmTngnDraw::IsProgressiveViewFollowingFrame() const
{
	return m_pShaderImpl->IsProgressiveViewFollowingFrame();
}

////////////////////////////////////////////////////////////////////////////////

void YmTngnDraw::SetModelMatrix(const DirectX::XMFLOAT4X4& matrix)
{
	m_pShaderImpl->SetModelMatrix(matrix);
}

double YmTngnDraw::GetPerspectiveViewNearZ()
{
	return m_pShaderImpl->GetPerspectiveViewNearZ();
}

DirectX::XMMATRIX YmTngnDraw::GetModelToViewMatrix()
{
	return m_pShaderImpl->GetModelToViewMatrix();
}

DirectX::XMMATRIX YmTngnDraw::GetModelToProjectionMatrix()
{
	return m_pShaderImpl->GetModelToProjectionMatrix();
}

D3DBufferPtr YmTngnDraw::CreateVertexBufferWithSize(UINT bufferByte, const void* aVertex, bool writeByCpu)
{
	D3D11_BUFFER_DESC hBufferDesc;
	ZeroMemory(&hBufferDesc, sizeof(hBufferDesc));
	hBufferDesc.ByteWidth = bufferByte;
	hBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	if (writeByCpu) {
		hBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		hBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	}
	else {
		hBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		hBufferDesc.CPUAccessFlags = 0;
	}
	//hBufferDesc.MiscFlags = 0;
	//hBufferDesc.StructureByteStride = sizeof(float);

	D3D11_SUBRESOURCE_DATA hSubResourceData;
	if (aVertex) {
		ZeroMemory(&hSubResourceData, sizeof(hSubResourceData));
		hSubResourceData.pSysMem = aVertex;
		//hSubResourceData.SysMemPitch = 0;
		//hSubResourceData.SysMemSlicePitch = 0;
	}

	D3DBufferPtr pBuffer;
	HRESULT hr = m_pDevice->CreateBuffer(&hBufferDesc, aVertex ? &hSubResourceData : nullptr, &pBuffer);
	if (FAILED(hr)) {
		YM_THROW_ERROR("CreateBuffer");
	}
	return pBuffer;
}

////////////////////////////////////////////////////////////////////////////////

void YmTngnDraw::DrawPointList(const YmTngnPointListVertex aVertex[], size_t nVertex)
{
	m_pShaderImpl->PrepareShaderParam();
	DrawPointListWithTempBuffer(aVertex, nVertex);
}

void YmTngnDraw::DrawPointList(
	const D3DBufferPtr& pVertexBuf, size_t vertexSize, size_t nVertex
)
{
	m_pShaderImpl->PrepareShaderParam();
	m_pShaderImpl->DrawPointList(pVertexBuf, vertexSize, nVertex);
	m_nDrawnPoint += nVertex;
}

void YmTngnDraw::DrawPointListWithSingleScannerPosition(
	const YmTngnPointListVertex aVertex[], size_t nVertex, const YmVector3d& scannerPos
)
{
	m_pShaderImpl->PrepareShaderParamToDrawPointListWithSingleScannerPosition(scannerPos);
	DrawPointListWithTempBuffer(aVertex, nVertex);
}

void YmTngnDraw::DrawPointListWithSingleScannerPosition(
	const D3DBufferPtr& pVertexBuf, size_t vertexSize, size_t nVertex, const YmVector3d& scannerPos
)
{
	m_pShaderImpl->PrepareShaderParamToDrawPointListWithSingleScannerPosition(scannerPos);
	m_pShaderImpl->DrawPointList(pVertexBuf, vertexSize, nVertex);
	m_nDrawnPoint += nVertex;
}

////////////////////////////////////////////////////////////////////////////////

YmDx11BufferWithSize YmTngnDraw::PrepareTempVertexBuffer()
{
	YmDx11BufferWithSize buffer = m_pShaderImpl->GetTempVertexBuffer();
	if (!buffer.pBuffer) {
		buffer.nBufferByte = 1024 * 1024 * sizeof(YmTngnPointListVertex);
		buffer.pBuffer = CreateVertexBufferWithSize(buffer.nBufferByte, nullptr, true);
		m_pShaderImpl->SetTempVertexBuffer(buffer);
	}
	return buffer;
}

void YmTngnDraw::DrawPointListWithTempBuffer(const YmTngnPointListVertex aVertex[], size_t nVertex)
{
	YmDx11BufferWithSize vertexBuffer = PrepareTempVertexBuffer();
	const size_t vertexSize = sizeof(YmTngnPointListVertex);
	const size_t nVertexInBufUpperBound = vertexBuffer.nBufferByte / vertexSize;
	YM_IS_TRUE(0 < nVertexInBufUpperBound);

	size_t nRemainingVertex = nVertex;
	while (0 < nRemainingVertex) {
		size_t nVertexInBuf = min(nVertexInBufUpperBound, nRemainingVertex);

		{
			YmDx11MappedSubResource mappedMemory = m_pShaderImpl->MapDynamicBuffer(vertexBuffer.pBuffer);
			UINT dataSize = static_cast<UINT>(nVertexInBuf * vertexSize);
			mappedMemory.Write(aVertex + (nVertex - nRemainingVertex), dataSize);
		}
		m_pShaderImpl->DrawPointList(vertexBuffer.pBuffer, vertexSize, nVertexInBuf);

		nRemainingVertex -= nVertexInBuf;
	}

	m_nDrawnPoint += nVertex;
}

////////////////////////////////////////////////////////////////////////////////