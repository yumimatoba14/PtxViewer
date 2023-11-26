#pragma once

#include "YmTngn.h"
#include "YmTngnShaderContext.h"
#include "YmDx11MappedSubResource.h"
#include "YmTngn2DText.h"
#include <vector>

namespace Ymcpp {

/// <summary>
/// This class has data related to shaders, which are shared among YmTngnViewModel and YmTngnDraw.
/// </summary>
class YmTngnShaderImpl
{
public:
	// XMFLOAT4X4 is used as type of member variables.
	using XMFLOAT4X4 = DirectX::XMFLOAT4X4;
	using XMFLOAT3 = DirectX::XMFLOAT3;

	// XMMATRIX is used as calculated values and so on.
	using XMMATRIX = DirectX::XMMATRIX;

	// Remarks:
	// XMLFLAT4x4 is row majar and post-multiplied by a vector.
	struct ShaderParam {
		XMFLOAT4X4 viewMatrix;
		XMFLOAT4X4 projectionMatrix;
		float pointSizeX;
		float pointSizeY;
		float pixelSizeX;
		float pixelSizeY;
		XMFLOAT3 scannerPosition;	// scanner point of ptx.
		float scannerDistanceUpperBound;
		float scannerDistanceDepthOffset;
		int isUseScannerPosition;
	};
public:
	explicit YmTngnShaderImpl(const YmTngnViewConfig& config, const D3DDevicePtr& pDevice, const D3DDeviceContextPtr& pDc);
	virtual ~YmTngnShaderImpl();

	YmTngnShaderImpl(const YmTngnShaderImpl& other) = delete;
	YmTngnShaderImpl(YmTngnShaderImpl&& other) noexcept = delete;

	YmTngnShaderImpl& operator = (const YmTngnShaderImpl& other) = delete;
	YmTngnShaderImpl& operator = (YmTngnShaderImpl&& other) noexcept = delete;

	bool IsNeedUpdateShaderParam() const { return m_isNeedUpdateShaderParam; }

	YmViewOp& GetViewOp();

	bool IsProgressiveViewMode() const { return m_isProgressiveViewMode; }
	bool IsProgressiveViewFollowingFrame() const { return m_isProgressiveViewFollowingFrame; }
	void SetProgressiveViewMode(bool enableProgressiveView, bool isFollowingFrame);

	YmVector2i GetViewSize() const { return m_viewSize; }
	void SetViewSize(const YmVector2i& viewSize) { m_viewSize = viewSize; m_isNeedUpdateShaderParam = true; }

	double GetPointSize() const { return m_pointSize; }
	void SetPointSize(double pointSize) { m_pointSize = pointSize; m_isNeedUpdateShaderParam = true; }

	void SetModelMatrix(const XMFLOAT4X4& matrix) { m_modelMatrix = matrix; m_isNeedUpdateShaderParam = true; }

	double GetPerspectiveViewNearZ() { return m_viewNearZ; }
	void SetPerspectiveViewNearZ(double z) { m_viewNearZ = z; m_isNeedUpdateShaderParam = true; }

	double GetPerspectiveViewFarZ() { return m_viewFarZ; }
	void SetPerspectiveViewFarZ(double z) { m_viewFarZ = z; m_isNeedUpdateShaderParam = true; }

	XMMATRIX GetModelToViewMatrix() const;

	XMMATRIX GetModelToProjectionMatrix() const;

	void PrepareShaderParam()
	{
		if (m_isNeedUpdateShaderParam) {
			UpdateShaderParam();
		}
	}

	void PrepareShaderParamToDrawPointListWithSingleScannerPosition(const YmVector3d& scannerPosition)
	{
		m_scannerPosition = scannerPosition;
		m_isUseScannerPosition = true;
		UpdateShaderParam();
		m_isUseScannerPosition = false;
	}

	YmDx11MappedSubResource MapDynamicBuffer(const D3DBufferPtr& pDynamicBuffer);
	YmDx11MappedSubResource MapResource(const D3DResourcePtr& pResource, D3D11_MAP mapType);

	YmDx11BufferWithSize GetTempVertexBuffer(int i) const {
		if (size_t(i) < m_tempVertexBufferList.size()) {
			return m_tempVertexBufferList.at(i);
		}
		return YmDx11BufferWithSize{ nullptr, 0 };
	}
	void SetTempVertexBuffer(int i, const YmDx11BufferWithSize& buffer) {
		if (m_tempVertexBufferList.size() <= size_t(i)) {
			m_tempVertexBufferList.resize(i + 1);
		}
		m_tempVertexBufferList[i] = buffer;
	}

	YmTngnDmDrawableObjectListPtr GetTransparentObjectList() const { return m_pTransparentObjectList; }
	void ClearTransparentObject();
	void RegisterTransparentObject(const YmTngnModelMatrixPtr& pMatrix, const YmTngnDrawableObjectPtr& pObject);

	const std::vector<YmTngn2DText>* Get2DTextList() const { return &m_2dTextList; }
	void Clear2DText();
	void Register2DText(LPCWSTR pText, const YmVector2i& origin);
	void Register3DText(const YmVector3d& origin, const std::string& text);

	void DrawPointList(
		const D3DBufferPtr& pVertexBuf, size_t vertexSize, size_t nVertex
	);

	void DrawPickablePointList(
		const D3DBufferPtr& pVertexBuf, const D3DBufferPtr& pPickIdBuf, size_t nVertex
	);

	void DrawTriangleList(
		const D3DBufferPtr& pVertexBuf, const D3DBufferPtr& pIndexBuf, size_t nIndex
	);

	void DrawLineList(
		const D3DBufferPtr& pVertexBuf, size_t nVertex
	);

private:
	void Initialize();
	void UpdateShaderParam();

	double GetAspectRatio() const;

	XMMATRIX GetProjectionMatrix(double aspectRatio) const;

	bool ConvertGlobalCoordToViewIntCoord(const YmVector3d& global, YmVector2i* pViewCoord) const;

	void InitializeShaderContextsForNormalRendering();
	void InitializeShaderContextsForPickableRendering();
	void InitializeShaderContextsForTriangleListNormalRendering();
	void InitializeShaderContextsForLineListNormalRendering();

	static YmTString GetHslsFilePath(const YmTString& fileName);

	D3DBufferPtr CreateConstantBuffer(size_t nByte);

	template<class T>
	void SetConstantBufferData(const D3DBufferPtr& pCBuffer, const T& data) {
		m_pDc->UpdateSubresource(pCBuffer.Get(), 0, nullptr, &data, 0, 0);
	}

	void SetShaderContext(const YmTngnShaderContext& context);
private:
	D3DInputLayoutPtr CreateInputLayout(
		const D3D11_INPUT_ELEMENT_DESC* aElement, UINT nElement,
		const YmTString& fileName, const std::string& entryPoint, const D3D_SHADER_MACRO* aMacro
	);
	D3DVertexShaderPtr CreateVertexShader(
		const YmTString& fileName, const std::string& entryPoint, const D3D_SHADER_MACRO* aMacro
	);
	D3DGeometryShaderPtr CreateGeometryShader(
		const YmTString& fileName, const std::string& entryPoint, const D3D_SHADER_MACRO* aMacro
	);
	D3DPixelShaderPtr CreatePixelShader(
		const YmTString& fileName, const std::string& entryPoint, const D3D_SHADER_MACRO* aMacro
	);

private:
	D3DDevicePtr m_pDevice;
	D3DDeviceContextPtr m_pDc;
	D3DBufferPtr m_pShaderParamConstBuf;
	YmTngnShaderContext m_pointListSc;
	YmTngnShaderContext m_pickablePointListSc;
	YmTngnShaderContext m_triangleListSc;
	YmTngnShaderContext m_lineListSc;
	YmVector2i m_viewSize;
	double m_pointSize;
	double m_fovAngleYDeg;
	double m_viewNearZ;
	double m_viewFarZ;
	YmVector3d m_scannerPosition;
	double m_scannerDistanceUpperBound;
	double m_scannerDistanceDepthOffset;
	std::unique_ptr<YmViewOp> m_pViewOp;
	XMFLOAT4X4 m_modelMatrix;	// pre multiplication
	bool m_isNeedUpdateShaderParam = true;
	bool m_isUseScannerPosition = false;
	bool m_isProgressiveViewMode = false;
	bool m_isProgressiveViewFollowingFrame = false;

	std::vector<YmDx11BufferWithSize> m_tempVertexBufferList;
	YmTngnDmDrawableObjectListPtr m_pTransparentObjectList;
	std::vector<YmTngn2DText> m_2dTextList;
};

}
