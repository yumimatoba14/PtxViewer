#pragma once

#include "YmTngnFwd.h"
#include "YmBase/YmVector.h"

namespace Ymcpp {

class YmTngnShaderImpl;
class YmTngnDmPtxFiles;

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

	void SetContent(const YmTngnDrawingModelPtr& pContent) { m_pContent = pContent; m_isNeedDraw = true; }
	YmTngnDmPtxFiles* PreparePtxFileContent();

	bool IsViewContentUpdated() const { return m_isViewUpdated; }
private:
	void SetupDevice(HWND hWnd, const YmVector2i& viewSize);
	void PrepareDepthStencilView();
	void PrepareRenderTargetView();

	void BeginDraw(bool isEraseBackground);
	void EndDraw();

private:
	std::unique_ptr< YmTngnViewConfig> m_pConfig;
	D3DDevicePtr m_pDevice;
	D3DDeviceContextPtr m_pDc;
	DXGISwapChainPtr m_pSwapChain;
	D3DDepthStencilStatePtr m_pDepthStencilState;
	D3DRasterizerStatePtr m_pRasterizerState;

	D3DDepthStencilViewPtr m_pDepthStencilView;
	D3DRenderTargetViewPtr m_pRenderTargetView;
	D3D11_VIEWPORT m_viewport = { 0, 0, 0, 0, 0, 0 };

	std::unique_ptr<YmTngnShaderImpl> m_pShaderImpl;
	bool m_isNeedDraw = false;
	bool m_isViewUpdated = false;

private:
	YmTngnDrawingModelPtr m_pContent;
	std::shared_ptr<YmTngnDmPtxFiles> m_pDmPtxFiles;
};

}
