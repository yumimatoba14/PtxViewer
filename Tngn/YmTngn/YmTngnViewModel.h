#pragma once

#include "YmTngnFwd.h"
#include "YmBase/YmVector.h"
#include <vector>

namespace Ymcpp {

class YmTngnShaderImpl;
class YmTngnDmObjFiles;
class YmTngnDmPtxFiles;
class YmTngnDmMemoryPointList;
class YmTextDrawerImpl;

class YmTngnViewModel
{
public:
	explicit YmTngnViewModel(const YmTngnViewConfig& config);
	virtual ~YmTngnViewModel();

	YmTngnViewModel(const YmTngnViewModel& other) = delete;
	YmTngnViewModel(YmTngnViewModel&& other) noexcept = delete;

	YmTngnViewModel& operator = (const YmTngnViewModel& other) = delete;
	YmTngnViewModel& operator = (YmTngnViewModel&& other) noexcept = delete;

public:
	void Setup(HWND hWnd);
	void ResizeBuffer(const YmVector2i& size);
	void Draw();
	bool IsNeedDraw() const;

	YmViewOp& GetViewOp();
	bool IsProgressiveViewMode() const;
	bool IsProgressiveViewFollowingFrame() const;
	void SetProgressiveViewMode(bool enableProgressiveView, bool isFollowingFrame = false);

	bool IsViewContentUpdated() const { return m_isViewUpdated; }

	void SetContent(const YmTngnDrawingModelPtr& pContent) { m_pContent = pContent; m_isNeedDraw = true; }

	void SetSelectedContent(const YmTngnDrawingModelPtr& pContent) { m_pSelectedContent = pContent; m_isNeedDraw = true; }

	bool IsDrawWithScannerPosition() const;
	void SetDrawWithScannerPosition(bool isUse);

	bool IsPickEnabled() const { return m_isPickEnabled; }
	void SetPickEnabled(bool isEnabled);

	std::vector<YmTngnPickedPoint> TryToPickPoint(const YmVector2i& mousePos);

	bool SaveViewToFile(REFGUID targetFormat, LPCTSTR targetFilePath);

public:
	std::shared_ptr<YmTngnDmPtxFiles> PreparePtxFileContent();
	std::shared_ptr<YmTngnDmObjFiles> PrepareObjFileContent();
	std::shared_ptr<YmTngnDmMemoryPointList> PrepareSelectedPointList();

	void AddLengthDimension(const YmVector3d& point0, const YmVector3d& point1);
	void ClearLengthDimension();

private:
	void SetupDevice(HWND hWnd, const YmVector2i& viewSize);
	void PrepareDepthStencilView();
	void PrepareRenderTargetView();
	void PrepareRenderTargetViewForNormalRendering();
	void PrepareRenderTargetViewForPick();
	std::unique_ptr<YmTextDrawerImpl> CreateTextDrawerImpl();

	void BeginDraw(bool isEraseBackground, bool isUseLastRenderingResources);
	void EndDraw();
	void CaptureRenderTargetResourcesForProgressiveView();

private:
	std::unique_ptr<YmTngnViewConfig> m_pConfig;
	D3DDevicePtr m_pDevice;
	D3DDeviceContextPtr m_pDc;
	DXGISwapChainPtr m_pSwapChain;
	D3DDepthStencilStatePtr m_pDepthStencilState;
	D3DDepthStencilStatePtr m_pDepthStencilStateForForegroundDraw;
	D3DBlendStatePtr m_pBlendStateForTransparency;
	D3DRasterizerStatePtr m_pRasterizerState;

	D3DDepthStencilViewPtr m_pDepthStencilView;
	D3DRenderTargetViewPtr m_pRenderTargetViewForNormalRendering;
	D3DRenderTargetViewPtr m_pRenderTargetViewForPick;
	D3D11_VIEWPORT m_viewport = { 0, 0, 0, 0, 0, 0 };

	bool m_isLastRenderingResourcesCaptured = false;
	D3DResourcePtr m_apLastRenderingTextureForProgressiveView[2];
	D3DResourcePtr m_pLastRenderingDepthStencilTextureForProgressiveView;

	std::unique_ptr<YmTngnShaderImpl> m_pShaderImpl;
	std::unique_ptr<YmTextDrawerImpl> m_pTextDrawerImpl;
	bool m_isNeedDraw = false;
	bool m_isViewUpdated = false;
	bool m_isPickEnabled = false;

private:
	YmTngnDrawingModelPtr m_pContent;
	YmTngnDrawingModelPtr m_pSelectedContent;
	std::shared_ptr<YmTngnDmPtxFiles> m_pDmPtxFiles;
	std::shared_ptr<YmTngnDmObjFiles> m_pDmObjFiles;
	std::shared_ptr<YmTngnDmMemoryPointList> m_pSelectedPoints;
	std::vector<YmTngnDmLengthDimensionPtr> m_lengthDimensions;
};

}
