#include "pch.h"
#include "YmTngnDraw.h"
#include "YmTngnShaderImpl.h"
#include "YmDx11MappedSubResource.h"

using namespace std;
using namespace Ymcpp;

static constexpr int TEMP_VERTEX_BUF_VERTEX = 0;
static constexpr int TEMP_VERTEX_BUF_PICK_ID = 1;

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

D3DBufferPtr YmTngnDraw::CreateInexBuffer(const UINT aIndex[], UINT nIndex)
{
	D3D11_BUFFER_DESC hBufferDesc;
	ZeroMemory(&hBufferDesc, sizeof(hBufferDesc));
	hBufferDesc.ByteWidth = sizeof(UINT) * nIndex;
	hBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	hBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	hBufferDesc.CPUAccessFlags = 0;
	//hBufferDesc.MiscFlags = 0;
	//hBufferDesc.StructureByteStride = sizeof(float);

	D3D11_SUBRESOURCE_DATA hSubResourceData;
	if (aIndex) {
		ZeroMemory(&hSubResourceData, sizeof(hSubResourceData));
		hSubResourceData.pSysMem = aIndex;
		//hSubResourceData.SysMemPitch = 0;
		//hSubResourceData.SysMemSlicePitch = 0;
	}

	D3DBufferPtr pBuffer;
	HRESULT hr = m_pDevice->CreateBuffer(&hBufferDesc, aIndex ? &hSubResourceData : nullptr, &pBuffer);
	if (FAILED(hr)) {
		YM_THROW_ERROR("CreateBuffer");
	}
	return pBuffer;
}

////////////////////////////////////////////////////////////////////////////////

void YmTngnDraw::RegisterTransparentObject(
	const YmTngnModelMatrixPtr& pMatrix, const YmTngnDrawableObjectPtr& pObject
)
{
	m_pShaderImpl->RegisterTransparentObject(pMatrix, pObject);
}

void YmTngnDraw::Register3DText(const YmVector3d& origin, const std::string& text)
{
	m_pShaderImpl->Register3DText(origin, text);
}

////////////////////////////////////////////////////////////////////////////////

void YmTngnDraw::DrawPointList(const YmTngnPointListVertex aVertex[], size_t nVertex, YmTngnPickTargetId firstId)
{
	m_pShaderImpl->PrepareShaderParam();
	DrawPointListWithTempBuffer(aVertex, nVertex, firstId);
}

void YmTngnDraw::DrawPointList(
	const D3DBufferPtr& pVertexBuf, size_t vertexSize, size_t nVertex, YmTngnPickTargetId firstId
)
{
	m_pShaderImpl->PrepareShaderParam();
	DrawPointListImpl(pVertexBuf, nVertex, firstId);
}

void YmTngnDraw::DrawPointListWithSingleScannerPosition(
	const YmTngnPointListVertex aVertex[], size_t nVertex, const YmVector3d& scannerPos, YmTngnPickTargetId firstId
)
{
	m_pShaderImpl->PrepareShaderParamToDrawPointListWithSingleScannerPosition(scannerPos);
	DrawPointListWithTempBuffer(aVertex, nVertex, firstId);
}

void YmTngnDraw::DrawPointListWithSingleScannerPosition(
	const D3DBufferPtr& pVertexBuf, size_t vertexSize, size_t nVertex, const YmVector3d& scannerPos, YmTngnPickTargetId firstId
)
{
	m_pShaderImpl->PrepareShaderParamToDrawPointListWithSingleScannerPosition(scannerPos);
	DrawPointListImpl(pVertexBuf, nVertex, firstId);
}

void YmTngnDraw::DrawTriangleList(
	const D3DBufferPtr& pVertexBuf, const D3DBufferPtr& pIndexBuf, size_t nIndex
)
{
	m_pShaderImpl->PrepareShaderParam();
	m_pShaderImpl->DrawTriangleList(pVertexBuf, pIndexBuf, nIndex);
}

void YmTngnDraw::DrawLineList(
	const YmTngnPointListVertex aVertex[], size_t nVertex
)
{
	m_pShaderImpl->PrepareShaderParam();
	YmDx11BufferWithSize vertexBuffer = PrepareTempVertexBuffer();
	const size_t vertexSize = sizeof(YmTngnPointListVertex);
	const size_t nVertexInBufUpperBound = vertexBuffer.nBufferByte / vertexSize;
	YM_IS_TRUE(1 < nVertexInBufUpperBound);

	size_t nRemainingVertex = nVertex;
	while (0 < nRemainingVertex) {
		size_t nVertexInBuf = min(nVertexInBufUpperBound, nRemainingVertex);
		nVertexInBuf = (nVertexInBuf / 2) * 2;
		if (nVertexInBuf == 0) {
			break;
		}

		{
			YmDx11MappedSubResource mappedMemory = m_pShaderImpl->MapDynamicBuffer(vertexBuffer.pBuffer);
			UINT dataSize = static_cast<UINT>(nVertexInBuf * vertexSize);
			mappedMemory.Write(aVertex + (nVertex - nRemainingVertex), dataSize);
		}
		m_pShaderImpl->DrawLineList(vertexBuffer.pBuffer, nVertexInBuf);

		nRemainingVertex -= nVertexInBuf;
	}
}

////////////////////////////////////////////////////////////////////////////////

void YmTngnDraw::DrawPointListImpl(
	const D3DBufferPtr& pVertexBuf, size_t nVertex, YmTngnPickTargetId firstId
)
{
	const bool isPickMode = (firstId != YM_TNGN_PICK_TARGET_NULL);

	if (isPickMode) {
		YmDx11BufferWithSize pickIdBuffer = m_pShaderImpl->GetTempVertexBuffer(TEMP_VERTEX_BUF_PICK_ID);

		const size_t nNecessaryBufByte = nVertex * sizeof(YmTngnPickTargetId);
		if (UINT_MAX < nNecessaryBufByte) {
			YM_THROW_ERROR("nVertex is too large. (Not supported case.)");
		}
		if (!pickIdBuffer.pBuffer || pickIdBuffer.nBufferByte < nNecessaryBufByte) {
			pickIdBuffer.nBufferByte = static_cast<UINT>(nNecessaryBufByte);
			pickIdBuffer.pBuffer = CreateVertexBufferWithSize(pickIdBuffer.nBufferByte, nullptr, true);
			m_pShaderImpl->SetTempVertexBuffer(TEMP_VERTEX_BUF_PICK_ID, pickIdBuffer);
		}

		{
			YmDx11MappedSubResource mappedMemory = m_pShaderImpl->MapDynamicBuffer(pickIdBuffer.pBuffer);
			YmTngnPickTargetId* aIdInBuf = mappedMemory.ToArray<YmTngnPickTargetId>(0);
			for (size_t iVertexInBuf = 0; iVertexInBuf < nVertex; ++iVertexInBuf) {
				YmTngnPickTargetId pickId = firstId + iVertexInBuf;
				aIdInBuf[iVertexInBuf] = pickId;
			}
		}

		m_pShaderImpl->DrawPickablePointList(pVertexBuf, pickIdBuffer.pBuffer, nVertex);
	}
	else {
		m_pShaderImpl->DrawPointList(pVertexBuf, sizeof(YmTngnPointListVertex), nVertex);
	}

	m_nDrawnPoint += nVertex;
}

YmDx11BufferWithSize YmTngnDraw::PrepareTempVertexBuffer()
{
	int bufIndex = 0;
	YmDx11BufferWithSize buffer = m_pShaderImpl->GetTempVertexBuffer(TEMP_VERTEX_BUF_VERTEX);
	if (!buffer.pBuffer) {
		buffer.nBufferByte = 1024 * 1024 * sizeof(YmTngnPointListVertex);
		buffer.pBuffer = CreateVertexBufferWithSize(buffer.nBufferByte, nullptr, true);
		m_pShaderImpl->SetTempVertexBuffer(TEMP_VERTEX_BUF_VERTEX, buffer);
	}
	return buffer;
}

void YmTngnDraw::DrawPointListWithTempBuffer(const YmTngnPointListVertex aVertex[], size_t nVertex, YmTngnPickTargetId firstId)
{
	const bool isPickMode = (firstId != YM_TNGN_PICK_TARGET_NULL);
	YmDx11BufferWithSize vertexBuffer = PrepareTempVertexBuffer();
	const size_t vertexSize = sizeof(YmTngnPointListVertex);
	const size_t nVertexInBufUpperBound = vertexBuffer.nBufferByte / vertexSize;
	YM_IS_TRUE(0 < nVertexInBufUpperBound);

	YmDx11BufferWithSize pickIdBuffer = m_pShaderImpl->GetTempVertexBuffer(TEMP_VERTEX_BUF_PICK_ID);
	if (isPickMode) {
		const size_t nNecessaryBufByte = nVertexInBufUpperBound * sizeof(YmTngnPickTargetId);
		YM_ASSERT(nNecessaryBufByte <= UINT_MAX);
		if (!pickIdBuffer.pBuffer || pickIdBuffer.nBufferByte < nNecessaryBufByte) {
			pickIdBuffer.nBufferByte = static_cast<UINT>(nNecessaryBufByte);
			pickIdBuffer.pBuffer = CreateVertexBufferWithSize(pickIdBuffer.nBufferByte, nullptr, true);
			m_pShaderImpl->SetTempVertexBuffer(TEMP_VERTEX_BUF_PICK_ID, pickIdBuffer);
		}
	}

	size_t nRemainingVertex = nVertex;
	while (0 < nRemainingVertex) {
		size_t nVertexInBuf = min(nVertexInBufUpperBound, nRemainingVertex);

		{
			YmDx11MappedSubResource mappedMemory = m_pShaderImpl->MapDynamicBuffer(vertexBuffer.pBuffer);
			UINT dataSize = static_cast<UINT>(nVertexInBuf * vertexSize);
			mappedMemory.Write(aVertex + (nVertex - nRemainingVertex), dataSize);
		}
		if (!isPickMode) {
			m_pShaderImpl->DrawPointList(vertexBuffer.pBuffer, vertexSize, nVertexInBuf);
		}
		else {
			{
				YmDx11MappedSubResource mappedMemory = m_pShaderImpl->MapDynamicBuffer(pickIdBuffer.pBuffer);
				YmTngnPickTargetId* aIdInBuf = mappedMemory.ToArray<YmTngnPickTargetId>(0);
				for (size_t iVertexInBuf = 0; iVertexInBuf < nVertexInBuf; ++iVertexInBuf) {
					YmTngnPickTargetId pickId = firstId + nVertex - nRemainingVertex + iVertexInBuf;
					aIdInBuf[iVertexInBuf] = pickId;
				}
			}

			m_pShaderImpl->DrawPickablePointList(vertexBuffer.pBuffer, pickIdBuffer.pBuffer, nVertexInBuf);
		}

		nRemainingVertex -= nVertexInBuf;
	}

	m_nDrawnPoint += nVertex;
}

////////////////////////////////////////////////////////////////////////////////
