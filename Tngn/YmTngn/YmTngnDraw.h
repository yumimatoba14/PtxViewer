#pragma once

#include "YmTngnFwd.h"
#include "YmTngnVectorUtil.h"

namespace Ymcpp {

class YmTngnShaderImpl;

struct YmTngnPointListVertex
{
	DirectX::XMFLOAT3 position;
	UINT rgba;
};

/// <summary>
/// This class has operations to implement YmTngnDrawingModel.
/// </summary>
class YmTngnDraw
{
public:
	YmTngnDraw(YmTngnShaderImpl* pShaderImpl, const D3DDevicePtr& pDevice)
		: m_pShaderImpl(pShaderImpl), m_pDevice(pDevice)
	{}
	/*virtual*/ ~YmTngnDraw();

	YmTngnDraw(const YmTngnDraw& other) = delete;
	YmTngnDraw(YmTngnDraw&& other) noexcept = delete;

	YmTngnDraw& operator = (const YmTngnDraw& other) = delete;
	YmTngnDraw& operator = (YmTngnDraw&& other) noexcept = delete;

public:
	bool IsProgressiveViewMode() const;
	bool IsProgressiveViewFollowingFrame() const;
	int64_t GetDrawnPointCount() const { return m_nDrawnPoint; }

public:
	void SetModelMatrix(const DirectX::XMFLOAT4X4& matrix);

	double GetPerspectiveViewNearZ();

	DirectX::XMMATRIX GetModelToViewMatrix();
	DirectX::XMMATRIX GetModelToProjectionMatrix();

	template<class T>
	D3DBufferPtr CreateVertexBuffer(const T* aVertex, UINT nVertex, bool writeByCpu)
	{
		return CreateVertexBufferWithSize(sizeof(T) * nVertex, aVertex, writeByCpu);
	}

	D3DBufferPtr CreateVertexBufferWithSize(UINT bufferByte, const void* aVertex, bool writeByCpu);

public:
	void DrawPointList(const YmTngnPointListVertex aVertex[], size_t nVertex);
	void DrawPointList(
		const D3DBufferPtr& pVertexBuf, size_t vertexSize, size_t nVertex
	);

	void DrawPointListWithSingleScannerPosition(
		const YmTngnPointListVertex aVertex[], size_t nVertex, const YmVector3d& scannerPos
	);
	void DrawPointListWithSingleScannerPosition(
		const D3DBufferPtr& pVertexBuf, size_t vertexSize, size_t nVertex, const YmVector3d& scannerPos
	);

private:
	YmDx11BufferWithSize PrepareTempVertexBuffer();
	void DrawPointListWithTempBuffer(const YmTngnPointListVertex aVertex[], size_t nVertex);

private:
	YmTngnShaderImpl* m_pShaderImpl;
	D3DDevicePtr m_pDevice;
	int64_t m_nDrawnPoint = 0;
};

}
