#include "pch.h"
#include "YmTngnViewModel.h"
#include "YmTngnDraw.h"
#include "YmTngnDrawingModel.h"
#include "YmTngnDmMemoryPointList.h"
#include "YmTngnDmPtxFiles.h"
#include "YmTngnDmDrawableObjectList.h"
#include "YmTngnDmLengthDimension.h"
#include "YmTngnShaderImpl.h"
#include "YmTngnViewConfig.h"
#include "YmTextDrawerImpl.h"
#include "ScreenGrab.h"

using namespace std;
using namespace Ymcpp;

////////////////////////////////////////////////////////////////////////////////

YmTngnViewModel::YmTngnViewModel(const YmTngnViewConfig& config)
	: m_pConfig(make_unique<YmTngnViewConfig>(config))
{
}

YmTngnViewModel::~YmTngnViewModel()
{
}

////////////////////////////////////////////////////////////////////////////////

static DXGIAdapterPtr SelectAdapter()
{
	DXGIFactoryPtr pFactory;

	HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&pFactory));
	if (FAILED(hr))
	{
		YM_THROW_ERROR("CreateDXGIFactory");
	}

	// Find the best GPU Output.
	UINT iChosenAdapter = 0;
	const UINT maxAdapter = 100;
	size_t maxGpuMemorySize = 0;
	for (UINT i = 0; i < maxAdapter; i++)
	{
		DXGIAdapterPtr pAdd;
		hr = pFactory->EnumAdapters(i, &pAdd);
		if (FAILED(hr)) {
			break;
		}

		DXGI_ADAPTER_DESC adapterDesc;
		hr = pAdd->GetDesc(&adapterDesc);
		if (FAILED(hr)) {
			YM_THROW_ERROR("GetDesc");
		}

		wstring description = adapterDesc.Description;
		size_t videoCardMemorySize = adapterDesc.DedicatedVideoMemory;

		DXGIAdapterPtr pAdapter;
		DXGIOutputPtr pOutput;
		hr = pAdd->EnumOutputs(0, &pOutput);
		if (FAILED(hr))
		{
			continue;
		}

		UINT numModes = 0;
		hr = pOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL);
		if (FAILED(hr))
		{
			YM_IGNORE_ERROR("GetDisplayModeList");
			continue;
		}

		if (videoCardMemorySize > maxGpuMemorySize)
		{
			maxGpuMemorySize = videoCardMemorySize;
			iChosenAdapter = i;
		}
	}

	DXGIAdapterPtr pAddapter;
	hr = pFactory->EnumAdapters(iChosenAdapter, &pAddapter);
	if (FAILED(hr))
	{
		YM_THROW_ERROR("EnumAdapters");
	}
	return pAddapter;
}

void YmTngnViewModel::Setup(HWND hWnd)
{
	YM_IS_TRUE(hWnd != nullptr);
	YM_ASSERT(m_pConfig);
	RECT clientRect;
	BOOL ok = ::GetClientRect(hWnd, &clientRect);
	if (!ok) {
		YM_THROW_ERROR("GetClientRect");
	}

	YmVector2i viewSize = YmVectorUtil::Make(clientRect.right - clientRect.left, clientRect.bottom - clientRect.top);

	SetupDevice(hWnd, viewSize);
	m_pShaderImpl = make_unique<YmTngnShaderImpl>(*m_pConfig, m_pDevice, m_pDc);
	m_pShaderImpl->SetViewSize(viewSize);

	m_pTextDrawerImpl = CreateTextDrawerImpl();
}

void YmTngnViewModel::ResizeBuffer(const YmVector2i& size)
{
	if (m_pSwapChain) {
		// release related buffer objects.
		// See https://learn.microsoft.com/ja-jp/windows/win32/api/dxgi/nf-dxgi-idxgiswapchain-resizebuffers?source=recommendations
		// See https://www.sfpgmr.net/blog/entry/dxgi-resizebuffers%E3%81%99%E3%82%8B%E3%81%A8%E3%81%8D%E3%81%AB%E6%B0%97%E3%82%92%E3%81%A4%E3%81%91%E3%82%8B%E3%81%93%E3%81%A8.html
		m_pRenderTargetViewForNormalRendering.Reset();
		m_pRenderTargetViewForPick.Reset();
		m_pDepthStencilView.Reset();
		m_apLastRenderingTextureForProgressiveView[0].Reset();
		m_apLastRenderingTextureForProgressiveView[1].Reset();
		m_pLastRenderingDepthStencilTextureForProgressiveView.Reset();
		m_pTextDrawerImpl->OnResizeBuffer();

		HRESULT hr = m_pSwapChain->ResizeBuffers(
			0, 0, 0, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);
		if (FAILED(hr)) {
			YM_THROW_ERROR("ResizeBuffers");
		}

		m_viewport.Width = static_cast<FLOAT>(size[0]);
		m_viewport.Height = static_cast<FLOAT>(size[1]);

		DXGISurfacePtr pRenderTargetBuffer;
		hr = m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pRenderTargetBuffer));
		if (FAILED(hr)) {
			YM_THROW_ERROR("GetBuffer");
		}
		m_pTextDrawerImpl->RecreateRenderTarget(pRenderTargetBuffer);
	}
	m_pShaderImpl->SetViewSize(size);
}

void YmTngnViewModel::Draw()
{
	m_isNeedDraw = false;
	m_isViewUpdated = false;

	const bool isProgressiveViewMode = IsProgressiveViewMode();
	bool isEraseBackground = !(isProgressiveViewMode && IsProgressiveViewFollowingFrame());
	bool useCapturedRenderTargetResource = false;
	if (!isEraseBackground && m_pSelectedContent && m_isLastRenderingResourcesCaptured) {
		useCapturedRenderTargetResource = true;
	}
	BeginDraw(isEraseBackground, useCapturedRenderTargetResource);

	bool isContinueProgressiveView = false;
	if (m_pContent) {
		YmTngnDraw draw(m_pShaderImpl.get(), m_pDevice);
		m_pContent->SetPickEnabled(IsPickEnabled());
		m_pContent->Draw(&draw);
		m_isViewUpdated = m_isViewUpdated || 0 < draw.GetDrawnPointCount();
		isContinueProgressiveView = isProgressiveViewMode && 0 < draw.GetDrawnPointCount();
	}

	auto pTransparentObjectList = m_pShaderImpl->GetTransparentObjectList();
	bool isDrawTrnasparentPhase = pTransparentObjectList && 0 < pTransparentObjectList->GetCount();
	bool isDrawForegroundPhase = m_pSelectedContent || !m_lengthDimensions.empty();

	bool isCaptureRenderTargetResource = false;
	if (isContinueProgressiveView) {
		isCaptureRenderTargetResource = m_pSelectedContent || isDrawTrnasparentPhase;
	}
	if (isCaptureRenderTargetResource) {
		CaptureRenderTargetResourcesForProgressiveView();
	}
	m_isLastRenderingResourcesCaptured = isCaptureRenderTargetResource;

	// Transparency is enabled from here.
	if (isDrawTrnasparentPhase || m_pSelectedContent) {
		float aBlendFactor[4] = { 1, 1, 1, 1 };
		m_pDc->OMSetBlendState(m_pBlendStateForTransparency.Get(), aBlendFactor, 0xFFFFFFFF);
	}

	if (isDrawTrnasparentPhase) {
		YmTngnDraw draw(m_pShaderImpl.get(), m_pDevice);
		pTransparentObjectList->Draw(&draw);
	}

	if (isDrawForegroundPhase) {
		m_pDc->OMSetDepthStencilState(m_pDepthStencilStateForForegroundDraw.Get(), 1);
		YmTngnDraw draw(m_pShaderImpl.get(), m_pDevice);
		for (auto& pDim : m_lengthDimensions) {
			pDim->Draw(&draw);
		}
		if (m_pSelectedContent) {
			m_pSelectedContent->Draw(&draw);
		}
		// Selected content should not be considered where view is updated or not.
	}

	if (m_pTextDrawerImpl && !m_pShaderImpl->Get2DTextList()->empty()) {
		m_pTextDrawerImpl->BeginDraw();
		m_pTextDrawerImpl->SetTextColor(m_pConfig->GetRgbaValue(YmTngnViewConfig::DEFAULT_TEXT_FG_COLOR));
		m_pTextDrawerImpl->SetBackgroundColor(m_pConfig->GetRgbaValue(YmTngnViewConfig::DEFAULT_TEXT_BG_COLOR));
		auto p2DText = m_pShaderImpl->Get2DTextList();
		for (auto& text : *p2DText) {
			m_pTextDrawerImpl->DrawText(text.GetText().c_str(), text.GetOrigin());
		}
		m_pTextDrawerImpl->EndDraw();
	}

	EndDraw();
}

bool YmTngnViewModel::IsNeedDraw() const
{
	return m_isNeedDraw || m_pShaderImpl->IsNeedUpdateShaderParam();
}

YmViewOp& YmTngnViewModel::GetViewOp()
{
	return m_pShaderImpl->GetViewOp();
}

bool YmTngnViewModel::IsProgressiveViewMode() const
{
	return m_pShaderImpl->IsProgressiveViewMode();
}

bool YmTngnViewModel::IsProgressiveViewFollowingFrame() const
{
	return m_pShaderImpl->IsProgressiveViewFollowingFrame();
}

void YmTngnViewModel::SetProgressiveViewMode(bool enableProgressiveView, bool isFollowingFrame)
{
	m_pShaderImpl->SetProgressiveViewMode(enableProgressiveView, isFollowingFrame);
}

bool YmTngnViewModel::IsDrawWithScannerPosition() const
{
	if (m_pContent != m_pDmPtxFiles) {
		return false;
	}
	else {
		return m_pDmPtxFiles->IsDrawWithScannerPosition();
	}
}

void YmTngnViewModel::SetDrawWithScannerPosition(bool isUse)
{
	if (m_pContent == m_pDmPtxFiles) {
		m_pDmPtxFiles->SetDrawWithScannerPosition(isUse);
	}
}

void YmTngnViewModel::SetPickEnabled(bool isEnabled)
{
	if (!m_isPickEnabled && isEnabled) {
		// It is necessary to draw view after enabling pick because pick uses drawing result.
		m_isNeedDraw = true;
	}
	m_isPickEnabled = isEnabled;
	if (!m_isPickEnabled && m_pSelectedPoints) {
		if (0 < m_pSelectedPoints->GetPointCount()) {
			m_pSelectedPoints->ClearPoint();
			m_isNeedDraw = true;
		}
	}
}

static D3DTexture2DPtr CaptureRenderTargetStagingTexture(
	const D3DDevicePtr& pDevice, const D3DDeviceContextPtr& pDc, const D3DRenderTargetViewPtr& pRtView
)
{
	D3DResourcePtr pRtResource;
	pRtView->GetResource(&pRtResource);
	YM_IS_TRUE(pRtResource);

	D3D11_RESOURCE_DIMENSION resType = D3D11_RESOURCE_DIMENSION_UNKNOWN;
	pRtResource->GetType(&resType);
	if (resType != D3D11_RESOURCE_DIMENSION_TEXTURE2D) {
		YM_THROW_ERROR("resouce dimension must be 2D");
	}
	D3DTexture2DPtr pRtTexture;
	HRESULT hr = pRtResource->QueryInterface(IID_PPV_ARGS(&pRtTexture));
	if (FAILED(hr)) {
		YM_THROW_ERROR("QueryInterface");
	}

	D3D11_TEXTURE2D_DESC desc;
	pRtTexture->GetDesc(&desc);

	if (1 < desc.SampleDesc.Count) {
		YM_THROW_ERROR("Not supported case (1 < desc.SampleDesc.Count)");
	}

	D3DTexture2DPtr pStaging;
	if ((desc.Usage == D3D11_USAGE_STAGING) && (desc.CPUAccessFlags & D3D11_CPU_ACCESS_READ)) {
		pStaging = pRtTexture;
	}
	else {
		D3D11_TEXTURE2D_DESC stagingDesc = desc;
		stagingDesc.BindFlags = 0;
		stagingDesc.MiscFlags &= D3D11_RESOURCE_MISC_TEXTURECUBE;
		stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		stagingDesc.Usage = D3D11_USAGE_STAGING;

		hr = pDevice->CreateTexture2D(&stagingDesc, nullptr, &pStaging);
		if (FAILED(hr)) {
			YM_THROW_ERROR("CreateTexture2D");
		}
		YM_IS_TRUE(pStaging);

		pDc->CopyResource(pStaging.Get(), pRtResource.Get());
	}

	return pStaging;
}

static map<uint64_t, size_t> CountTexture2DColor(YmTngnShaderImpl* pShaderImpl, const D3DTexture2DPtr& pStaging)
{
	YM_NOEXCEPT_BEGIN("CountTexture2DColor");
	D3D11_TEXTURE2D_DESC desc;
	pStaging->GetDesc(&desc);

	YmDx11MappedSubResource mappedTexture = pShaderImpl->MapResource(pStaging, D3D11_MAP_READ);
	const int64_t imageSize = int64_t(mappedTexture.GetRowPitch()) * int64_t(desc.Height);
	YM_IS_TRUE(imageSize < UINT_MAX);
	const int pixelSize = mappedTexture.GetRowPitch() / desc.Width;
	YM_IS_TRUE(pixelSize <= 8);
	map<uint64_t, size_t> colorCounter;
	for (UINT row = 0; row < desc.Height; ++row) {
		for (UINT col = 0; col < desc.Width; ++col) {
			const unsigned char* pHead = mappedTexture.ToArray<unsigned char>(size_t(row) * mappedTexture.GetRowPitch() + col * pixelSize);
			uint64_t value = 0;
			for (int i = 0; i < pixelSize; ++i) {
				value += (uint64_t(pHead[i]) << (8 * i));
			}
			auto it = colorCounter.find(value);
			if (it == colorCounter.end()) {
				colorCounter[value] = 1;
			}
			else {
				it->second++;
			}
		}
	}

	return colorCounter;
	YM_NOEXCEPT_END;
	return map<uint64_t, size_t>();
}

static YmTngnPickTargetId GetTexture2DInt64Pixel(YmTngnShaderImpl* pShaderImpl, const D3DTexture2DPtr& pStaging, const YmVector2i& mousePos)
{
	D3D11_TEXTURE2D_DESC desc;
	pStaging->GetDesc(&desc);
	YM_ASSERT(0 <= mousePos[0] && UINT(mousePos[0]) < desc.Width);
	YM_ASSERT(0 <= mousePos[1] && UINT(mousePos[1]) < desc.Height);

	YmDx11MappedSubResource mappedTexture = pShaderImpl->MapResource(pStaging, D3D11_MAP_READ);
	const int64_t imageSize = int64_t(mappedTexture.GetRowPitch()) * int64_t(desc.Height);
	YM_ASSERT(imageSize < UINT_MAX);
	const int pixelSize = mappedTexture.GetRowPitch() / desc.Width;
	YM_IS_TRUE(pixelSize == 8);
	const unsigned char* pPixelHead = mappedTexture.ToArray<unsigned char>(size_t(mousePos[1]) * mappedTexture.GetRowPitch() + mousePos[0] * pixelSize);
	uint64_t value = 0;
	for (int i = 0; i < pixelSize; ++i) {
		value += (uint64_t(pPixelHead[i]) << (8 * i));
	}
	return YmTngnPickTargetId(value);
}

std::vector<YmTngnPointListVertex> YmTngnViewModel::TryToPickPoint(const YmVector2i& mousePos)
{
	D3DTexture2DPtr pStaging = CaptureRenderTargetStagingTexture(m_pDevice, m_pDc, m_pRenderTargetViewForPick);
	if (false) {
		CountTexture2DColor(m_pShaderImpl.get(), pStaging);
	}

	YmTngnPickTargetId pickedId = GetTexture2DInt64Pixel(m_pShaderImpl.get(), pStaging, mousePos);
	if (pickedId == YM_TNGN_PICK_TARGET_NULL) {
		return vector<YmTngnPointListVertex>();
	}
	return m_pContent->FindPickedPoints(pickedId);
}

/// <summary>
/// Save view image to file.
/// </summary>
/// <param name="targetFormat">GUID_ContainerFormatPng for example.</param>
/// <param name="targetFilePath"></param>
/// <returns>false if failed.</returns>
/// This function may throw an exception, also.
bool YmTngnViewModel::SaveViewToFile(REFGUID targetFormat, LPCTSTR targetFilePath)
{
	using namespace ATL;
	YM_IS_TRUE(targetFilePath != nullptr);
	YM_IS_TRUE(m_pRenderTargetViewForNormalRendering);

	D3DResourcePtr pResource;
	m_pRenderTargetViewForNormalRendering->GetResource(&pResource);
	YM_IS_TRUE(pResource);

	HRESULT hr = DirectX::SaveWICTextureToFile(
		m_pDc.Get(), pResource.Get(), targetFormat, CT2W(targetFilePath)
	);
	if (FAILED(hr)) {
		YM_IGNORE_ERROR("DirectX::SaveWICTextureToFile");
		return false;
	}
	return true;
}

////////////////////////////////////////////////////////////////////////////////

std::shared_ptr<YmTngnDmPtxFiles> YmTngnViewModel::PreparePtxFileContent()
{
	if (!m_pDmPtxFiles) {
		m_pDmPtxFiles = make_shared<YmTngnDmPtxFiles>(*m_pConfig);
	}
	SetContent(m_pDmPtxFiles);
	return m_pDmPtxFiles;
}

shared_ptr<YmTngnDmMemoryPointList> YmTngnViewModel::PrepareSelectedPointList()
{
	if (!m_pSelectedPoints) {
		m_pSelectedPoints = make_shared<YmTngnDmMemoryPointList>();
		m_pSelectedPoints->SetSupportProgressiveMode(false);
	}
	SetSelectedContent(m_pSelectedPoints);
	return m_pSelectedPoints;
}

void YmTngnViewModel::AddLengthDimension(const YmVector3d& point0, const YmVector3d& point1)
{
	auto pDim = make_shared<YmTngnDmLengthDimension>(point0, point1);
	m_lengthDimensions.push_back(pDim);
	m_isNeedDraw = true;
}

void YmTngnViewModel::ClearLengthDimension()
{
	m_lengthDimensions.clear();
	m_isNeedDraw = true;
}

////////////////////////////////////////////////////////////////////////////////

void YmTngnViewModel::SetupDevice(HWND hWnd, const YmVector2i& viewSize)
{
	DXGIAdapterPtr pAdapter = SelectAdapter();

	UINT cdev_flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef _DEBUG
	cdev_flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = viewSize[0];
	sd.BufferDesc.Height = viewSize[1];
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;	//1/60 = 60fps
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = true;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	D3D_FEATURE_LEVEL aFeatureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_1,
	};

	// m_pDC is Immediate Device Context.
	HRESULT hr = D3D11CreateDeviceAndSwapChain(pAdapter.Get(), D3D_DRIVER_TYPE_UNKNOWN, NULL,
		cdev_flags, aFeatureLevels, 1, D3D11_SDK_VERSION, &sd,
		&m_pSwapChain, &m_pDevice, NULL, &m_pDc);
	if (FAILED(hr)) {
		YM_THROW_ERROR("D3D11CreateDeviceAndSwapChain");
	}

	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	ZeroMemory(&depthStencilDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

	depthStencilDesc.StencilEnable = true;
	depthStencilDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	depthStencilDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;

	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	hr = m_pDevice->CreateDepthStencilState(&depthStencilDesc, &m_pDepthStencilState);
	if (FAILED(hr)) {
		YM_THROW_ERROR("CreateDepthStencilState");
	}

	D3D11_DEPTH_STENCIL_DESC depthStencilForFgDrawDesc = depthStencilDesc;
	depthStencilForFgDrawDesc.DepthEnable = FALSE;
	hr = m_pDevice->CreateDepthStencilState(&depthStencilForFgDrawDesc, &m_pDepthStencilStateForForegroundDraw);
	if (FAILED(hr)) {
		YM_THROW_ERROR("CreateDepthStencilState");
	}

	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(D3D11_BLEND_DESC));
	for (int i = 0; i < 8; ++i) {
		blendDesc.RenderTarget[i].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	}
	//blendDesc.AlphaToCoverageEnable = TRUE;
	blendDesc.IndependentBlendEnable = TRUE;	// TRUE to disable blending of target[1].
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	hr = m_pDevice->CreateBlendState(&blendDesc, &m_pBlendStateForTransparency);
	if (FAILED(hr)) {
		YM_THROW_ERROR("CreateBlendState");
	}

	PrepareDepthStencilView();
	PrepareRenderTargetView();

	// viewport
	m_viewport.Width = static_cast<FLOAT>(viewSize[0]);
	m_viewport.Height = static_cast<FLOAT>(viewSize[1]);
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;
	m_viewport.TopLeftX = 0;
	m_viewport.TopLeftY = 0;

	D3D11_RASTERIZER_DESC hRasterizerDesc = {
		D3D11_FILL_SOLID,
		D3D11_CULL_NONE,
		TRUE,
		0,
		0.0f,
		FALSE,
		FALSE,
		FALSE,
		FALSE,
		FALSE
	};
	hr = m_pDevice->CreateRasterizerState(&hRasterizerDesc, &m_pRasterizerState);
	if (FAILED(hr)) {
		YM_THROW_ERROR("CreateRasterizerState");
	}
}

void YmTngnViewModel::PrepareDepthStencilView()
{
	if (m_pDepthStencilView.Get()) {
		return;
	}

	DXGI_SWAP_CHAIN_DESC sd;
	HRESULT hr = m_pSwapChain->GetDesc(&sd);
	if (FAILED(hr)) {
		YM_THROW_ERROR("GetDesc");
	}

	D3D11_TEXTURE2D_DESC hTexture2dDesc;
	hTexture2dDesc.Width = sd.BufferDesc.Width;
	hTexture2dDesc.Height = sd.BufferDesc.Height;
	hTexture2dDesc.MipLevels = 1;
	hTexture2dDesc.ArraySize = 1;
	hTexture2dDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	hTexture2dDesc.SampleDesc = sd.SampleDesc;
	hTexture2dDesc.Usage = D3D11_USAGE_DEFAULT;
	hTexture2dDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	hTexture2dDesc.CPUAccessFlags = 0;
	hTexture2dDesc.MiscFlags = 0;
	D3DTexture2DPtr pDepthStencilBuffer;
	hr = m_pDevice->CreateTexture2D(&hTexture2dDesc, NULL, &pDepthStencilBuffer);
	if (FAILED(hr)) {
		YM_THROW_ERROR("CreateTexture2D");
	}

	D3D11_DEPTH_STENCIL_VIEW_DESC hDepthStencilViewDesc;
	hDepthStencilViewDesc.Format = hTexture2dDesc.Format;
	hDepthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
	hDepthStencilViewDesc.Flags = 0;
	hr = m_pDevice->CreateDepthStencilView(pDepthStencilBuffer.Get(), &hDepthStencilViewDesc, &m_pDepthStencilView);
	if (FAILED(hr)) {
		YM_THROW_ERROR("CreateDepthStencilState");
	}
}

void YmTngnViewModel::PrepareRenderTargetView()
{
	PrepareRenderTargetViewForNormalRendering();
	PrepareRenderTargetViewForPick();
}

void YmTngnViewModel::PrepareRenderTargetViewForNormalRendering()
{
	if (m_pRenderTargetViewForNormalRendering.Get()) {
		return;
	}

	D3DTexture2DPtr pRenderTargetBuffer;
	HRESULT hr = m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pRenderTargetBuffer));
	if (FAILED(hr)) {
		YM_THROW_ERROR("GetBuffer");
	}

	hr = m_pDevice->CreateRenderTargetView(pRenderTargetBuffer.Get(), NULL, &m_pRenderTargetViewForNormalRendering);
	if (FAILED(hr)) {
		YM_THROW_ERROR("CreateRenderTargetView");
	}
}

void YmTngnViewModel::PrepareRenderTargetViewForPick()
{
	if (m_pRenderTargetViewForPick.Get()) {
		return;
	}

	DXGI_SWAP_CHAIN_DESC sd;
	HRESULT hr = m_pSwapChain->GetDesc(&sd);
	if (FAILED(hr)) {
		YM_THROW_ERROR("GetDesc");
	}

	D3D11_TEXTURE2D_DESC hTexture2dDesc;
	hTexture2dDesc.Width = sd.BufferDesc.Width;
	hTexture2dDesc.Height = sd.BufferDesc.Height;
	hTexture2dDesc.MipLevels = 1;
	hTexture2dDesc.ArraySize = 1;
	hTexture2dDesc.Format = DXGI_FORMAT_R16G16B16A16_UINT;
	hTexture2dDesc.SampleDesc = sd.SampleDesc;
	hTexture2dDesc.Usage = D3D11_USAGE_DEFAULT;
	hTexture2dDesc.BindFlags = D3D11_BIND_RENDER_TARGET;
	hTexture2dDesc.CPUAccessFlags = 0;
	hTexture2dDesc.MiscFlags = 0;
	D3DTexture2DPtr pRenderTargetBuffer;
	hr = m_pDevice->CreateTexture2D(&hTexture2dDesc, NULL, &pRenderTargetBuffer);
	if (FAILED(hr)) {
		YM_THROW_ERROR("CreateTexture2D");
	}

	hr = m_pDevice->CreateRenderTargetView(pRenderTargetBuffer.Get(), NULL, &m_pRenderTargetViewForPick);
	if (FAILED(hr)) {
		YM_THROW_ERROR("CreateRenderTargetView");
	}
}

std::unique_ptr<YmTextDrawerImpl> YmTngnViewModel::CreateTextDrawerImpl()
{
	DXGISurfacePtr pRenderTargetBuffer;
	HRESULT hr = m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pRenderTargetBuffer));
	if (FAILED(hr)) {
		YM_THROW_ERROR("GetBuffer");
	}

	return make_unique<YmTextDrawerImpl>(pRenderTargetBuffer, m_pConfig.get());
}

////////////////////////////////////////////////////////////////////////////////

static void CopyResourceToView(
	const D3DDeviceContextPtr& pDc, const D3DResourcePtr& pSource, const D3DViewPtr& pView
)
{
	YM_IS_TRUE(pDc);
	if (!pSource || !pView) {
		return;
	}
	
	D3DResourcePtr pDestination;
	pView->GetResource(&pDestination);
	YM_IS_TRUE(pDestination);

	pDc->CopyResource(pDestination.Get(), pSource.Get());
}

void YmTngnViewModel::BeginDraw(bool isEraseBackground, bool isUseLastRenderingResources)
{
	PrepareDepthStencilView();
	PrepareRenderTargetView();

	if (isEraseBackground) {
		float aClearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f }; //red,green,blue,alpha
		m_pDc->ClearRenderTargetView(m_pRenderTargetViewForNormalRendering.Get(), aClearColor);
		// Default color shall be 0 in case of RTV for pick because D3D_SELECTION_TARGET_NULL is 0.
		// Transparency shall not be used, also.
		aClearColor[3] = 0.0f;
		m_pDc->ClearRenderTargetView(m_pRenderTargetViewForPick.Get(), aClearColor);
		m_pDc->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		m_pShaderImpl->ClearTransparentObject();
		m_pShaderImpl->Clear2DText();
	}
	else if (isUseLastRenderingResources) {
		CopyResourceToView(m_pDc, m_apLastRenderingTextureForProgressiveView[0], m_pRenderTargetViewForNormalRendering);
		CopyResourceToView(m_pDc, m_apLastRenderingTextureForProgressiveView[1], m_pRenderTargetViewForPick);
		CopyResourceToView(m_pDc, m_pLastRenderingDepthStencilTextureForProgressiveView, m_pDepthStencilView);
	}

	m_pDc->RSSetViewports(1, &m_viewport);
	m_pDc->RSSetState(m_pRasterizerState.Get());
	m_pDc->OMSetDepthStencilState(m_pDepthStencilState.Get(), 1);
	m_pDc->OMSetBlendState(nullptr, nullptr, 0xffffffff);

	ID3D11RenderTargetView* apRtv[2] = { m_pRenderTargetViewForNormalRendering.Get(), m_pRenderTargetViewForPick.Get() };
	m_pDc->OMSetRenderTargets(2, apRtv, m_pDepthStencilView.Get());
}

void YmTngnViewModel::EndDraw()
{
	HRESULT hr = m_pSwapChain->Present(0, 0);
	if (FAILED(hr)) {
		YM_IGNORE_ERROR("Present");
	}
}

static D3DResourcePtr CopyViewResource(
	const D3DDevicePtr& pDevice, const D3DDeviceContextPtr& pDc, const D3DViewPtr& pView, const D3DResourcePtr& pDestinationIn
)
{
	if (!pView) {
		return nullptr;
	}
	D3DResourcePtr pSource;
	pView->GetResource(&pSource);

	D3DResourcePtr pDestination = pDestinationIn;
	if (!pDestination) {
		D3D11_RESOURCE_DIMENSION resType = D3D11_RESOURCE_DIMENSION_UNKNOWN;
		pSource->GetType(&resType);
		if (resType == D3D11_RESOURCE_DIMENSION_TEXTURE2D) {
			D3DTexture2DPtr pSourceTexture;
			HRESULT hr = pSource->QueryInterface(IID_PPV_ARGS(&pSourceTexture));
			if (FAILED(hr)) {
				YM_THROW_ERROR("QueryInterface");
			}

			D3D11_TEXTURE2D_DESC desc;
			pSourceTexture->GetDesc(&desc);

			if (1 < desc.SampleDesc.Count) {
				YM_THROW_ERROR("Not supported case (1 < desc.SampleDesc.Count)");
			}

			D3DTexture2DPtr pDestinationTexture;
			hr = pDevice->CreateTexture2D(&desc, nullptr, &pDestinationTexture);
			if (FAILED(hr)) {
				YM_THROW_ERROR("CreateTexture2D");
			}
			YM_IS_TRUE(pDestinationTexture);

			pDestination = pDestinationTexture;
		}
		else {
			YM_THROW_ERROR("not supported resource");
		}
	}
	YM_IS_TRUE(pDestination);

	pDc->CopyResource(pDestination.Get(), pSource.Get());

	return pDestination;
}

void YmTngnViewModel::CaptureRenderTargetResourcesForProgressiveView()
{
	D3DRenderTargetViewPtr apRtView[2] = { m_pRenderTargetViewForNormalRendering, m_pRenderTargetViewForPick };
	for (int i = 0; i < 2; ++i) {
		m_apLastRenderingTextureForProgressiveView[i] = CopyViewResource(
			m_pDevice, m_pDc, apRtView[i], m_apLastRenderingTextureForProgressiveView[i]
		);
	}

	m_pLastRenderingDepthStencilTextureForProgressiveView = CopyViewResource(
		m_pDevice, m_pDc, m_pDepthStencilView, m_pLastRenderingDepthStencilTextureForProgressiveView
	);
}

////////////////////////////////////////////////////////////////////////////////
