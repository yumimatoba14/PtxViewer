#include "pch.h"
#include "YmTngnDraw.h"
#include "YmTngnShaderImpl.h"
#include "YmDx11MappedSubResource.h"

using namespace std;
using namespace Ymcpp;
using namespace DirectX;

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

D3DBufferPtr YmTngnDraw::CreateIndexBuffer(const UINT aIndex[], UINT nIndex)
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

YmTngnPickTargetId YmTngnDraw::MakePickTargetId(int64_t numId)
{
	return m_pShaderImpl->MakePickTargetId(numId);
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
	const D3DBufferPtr& pVertexBuf, const D3DBufferPtr& pIndexBuf, size_t nIndex,
	YmTngnPickTargetId pickId
)
{
	if (pickId == YM_TNGN_PICK_TARGET_NULL) {
		m_pShaderImpl->PrepareShaderParam();
		m_pShaderImpl->DrawTriangleList(pVertexBuf, pIndexBuf, nIndex);
	}
	else {
		m_pShaderImpl->SetShaderParamPickTargetId(pickId);
		m_pShaderImpl->PrepareShaderParam();
		m_pShaderImpl->DrawPickableTriangleList(pVertexBuf, pIndexBuf, nIndex);
	}
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

/// <summary>
/// Calculate min value of view depth for local AABB.
/// It is assumed that view coordiante system is right hand system and view depth direction is +Z.
/// </summary>
/// <param name="localAabb"></param>
/// <param name="localToViewMatrix"></param>
/// <returns></returns>
double YmTngnDraw::CalculateViewMinDepthForLocalBox(const YmAabBox3d& localAabb, const DirectX::XMMATRIX& localToViewMatrix)
{
	YM_IS_TRUE(localAabb.IsInitialized());
	double depth = DBL_MAX;
	for (int i = 0; i < 8; ++i) {
		float coord[3];
		coord[0] = static_cast<float>((i & 0x01) ? localAabb.GetMaxPoint()[0] : localAabb.GetMinPoint()[0]);
		coord[1] = static_cast<float>((i & 0x02) ? localAabb.GetMaxPoint()[1] : localAabb.GetMinPoint()[1]);
		coord[2] = static_cast<float>((i & 0x04) ? localAabb.GetMaxPoint()[2] : localAabb.GetMinPoint()[2]);

		XMVECTOR localVec = XMVectorSet(coord[0], coord[1], coord[2], 1);
		XMVECTOR viewVec = XMVector4Transform(localVec, localToViewMatrix);
		depth = min(depth, (double)XMVectorGetZ(viewVec));
	}
	return depth;
}

/// <summary>
/// 
/// </summary>
/// <param name="modelToViewMatrix">modelToViewMatrix is pre-multiplied matrix by model coordinates.</param>
/// <param name="aabb"></param>
/// <param name="distanceLBIn"></param>
/// <returns></returns>
static double CalcPointListEnumerationPrecision(
	const YmAabBox3d& localAabb, const XMMATRIX& localToViewMatrix, double distanceLBIn
)
{
	YM_IS_TRUE(localAabb.IsInitialized());
	const double distanceLB = max(0, distanceLBIn);
	double minDistance = DBL_MAX;
	for (int i = 0; i < 8; ++i) {
		float coord[3];
		coord[0] = static_cast<float>((i & 0x01) ? localAabb.GetMaxPoint()[0] : localAabb.GetMinPoint()[0]);
		coord[1] = static_cast<float>((i & 0x02) ? localAabb.GetMaxPoint()[1] : localAabb.GetMinPoint()[1]);
		coord[2] = static_cast<float>((i & 0x04) ? localAabb.GetMaxPoint()[2] : localAabb.GetMinPoint()[2]);

		XMVECTOR localVec = XMVectorSet(coord[0], coord[1], coord[2], 1);
		XMVECTOR viewVec = XMVector4Transform(localVec, localToViewMatrix);
		double value = XMVectorGetZ(viewVec);

		value *= -1;
		if (value < minDistance) {
			minDistance = value;
			if (minDistance < distanceLB) {
				minDistance = distanceLB;
				break;
			}
		}
	}
	return  0.001 * minDistance;  // TODO: fix me
}

double YmTngnDraw::EstimateLengthPerDotForLocalBox(const YmAabBox3d& localAabb)
{
	double persNearZ = GetPerspectiveViewNearZ();
	return CalcPointListEnumerationPrecision(localAabb, GetModelToViewMatrix(), persNearZ);
}

////////////////////////////////////////////////////////////////////////////////

void YmTngnDraw::DrawPointListImpl(
	const D3DBufferPtr& pVertexBuf, size_t nVertex, YmTngnPickTargetId firstId
)
{
	const bool isPickMode = (firstId != YM_TNGN_PICK_TARGET_NULL);

	if (isPickMode) {
		const size_t nNecessaryBufByte = nVertex * sizeof(YmTngnPickTargetId);
		if (UINT_MAX < nNecessaryBufByte) {
			YM_THROW_ERROR("nVertex is too large. (Not supported case.)");
		}
		YmDx11BufferWithSize pickIdBuffer = PrepareTempVertexBuffer(TEMP_VERTEX_BUF_PICK_ID, nNecessaryBufByte);

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

YmDx11BufferWithSize YmTngnDraw::PrepareTempVertexBuffer(int bufferId, size_t nNecessaryBufByte)
{
	YmDx11BufferWithSize buffer = m_pShaderImpl->GetTempVertexBuffer(bufferId);
	YM_ASSERT(nNecessaryBufByte <= UINT_MAX);
	if (!buffer.pBuffer || buffer.nBufferByte < nNecessaryBufByte) {
		buffer.nBufferByte = static_cast<UINT>(nNecessaryBufByte);
		buffer.pBuffer = CreateVertexBufferWithSize(buffer.nBufferByte, nullptr, true);
		m_pShaderImpl->SetTempVertexBuffer(bufferId, buffer);
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

	YmDx11BufferWithSize pickIdBuffer = { nullptr, 0 };
	if (isPickMode) {
		pickIdBuffer = PrepareTempVertexBuffer(TEMP_VERTEX_BUF_PICK_ID, nVertexInBufUpperBound * sizeof(YmTngnPickTargetId));
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
