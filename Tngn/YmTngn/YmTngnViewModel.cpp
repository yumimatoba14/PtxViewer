#include "pch.h"
#include "YmTngnViewModel.h"
#include "YmTngnDraw.h"
#include "YmTngnDrawingModel.h"
#include "YmTngnDmPtxFiles.h"
#include "YmTngnShaderImpl.h"
#include "YmTngnViewConfig.h"

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

	HRESULT hr = CreateDXGIFactory(__uuidof(IDXGIFactory), &pFactory);
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
}

void YmTngnViewModel::ResizeBuffer(const YmVector2i& size)
{
	if (m_pSwapChain) {
		// release related buffer objects.
		// See https://learn.microsoft.com/ja-jp/windows/win32/api/dxgi/nf-dxgi-idxgiswapchain-resizebuffers?source=recommendations
		// See https://www.sfpgmr.net/blog/entry/dxgi-resizebuffers%E3%81%99%E3%82%8B%E3%81%A8%E3%81%8D%E3%81%AB%E6%B0%97%E3%82%92%E3%81%A4%E3%81%91%E3%82%8B%E3%81%93%E3%81%A8.html
		m_pRenderTargetView.Reset();
		m_pDepthStencilView.Reset();

		HRESULT hr = m_pSwapChain->ResizeBuffers(
			0, 0, 0, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);
		if (FAILED(hr)) {
			YM_THROW_ERROR("ResizeBuffers");
		}

		m_viewport.Width = static_cast<FLOAT>(size[0]);
		m_viewport.Height = static_cast<FLOAT>(size[1]);
	}
	m_pShaderImpl->SetViewSize(size);
}

void YmTngnViewModel::Draw()
{
	m_isNeedDraw = false;

	bool isEraseBackground = !(IsProgressiveViewMode() && IsProgressiveViewFollowingFrame());
	BeginDraw(isEraseBackground);

	YmTngnDraw draw(m_pShaderImpl.get(), m_pDevice);
	if (m_pContent) {
		m_pContent->Draw(&draw);
	}

	EndDraw();
	m_isViewUpdated = 0 < draw.GetDrawnPointCount();
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

YmTngnDmPtxFiles* YmTngnViewModel::PreparePtxFileContent()
{
	if (!m_pDmPtxFiles) {
		m_pDmPtxFiles = make_shared<YmTngnDmPtxFiles>();
	}
	SetContent(m_pDmPtxFiles);
	return m_pDmPtxFiles.get();
}

////////////////////////////////////////////////////////////////////////////////

void YmTngnViewModel::SetupDevice(HWND hWnd, const YmVector2i& viewSize)
{
	DXGIAdapterPtr pAdapter = SelectAdapter();

	UINT cdev_flags = 0;
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
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;

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

#if 0
	D3D11_DEPTH_STENCIL_DESC depthStencilForSelectedEntityDesc = depthStencilDesc;
	depthStencilForSelectedEntityDesc.DepthEnable = FALSE;
	hr = m_pDevice->CreateDepthStencilState(&depthStencilForSelectedEntityDesc, &m_pDepthStencilStateForSelectedEntity);
	if (FAILED(hr)) {
		YM_THROW_ERROR("CreateDepthStencilState");
	}
#endif

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
	if (m_pRenderTargetView.Get()) {
		return;
	}

	D3DTexture2DPtr pRenderTargetBuffer;
	HRESULT hr = m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pRenderTargetBuffer));
	if (FAILED(hr)) {
		YM_THROW_ERROR("GetBuffer");
	}

	hr = m_pDevice->CreateRenderTargetView(pRenderTargetBuffer.Get(), NULL, &m_pRenderTargetView);
	if (FAILED(hr)) {
		YM_THROW_ERROR("CreateRenderTargetView");
	}
}

////////////////////////////////////////////////////////////////////////////////

void YmTngnViewModel::BeginDraw(bool isEraseBackground)
{
	PrepareDepthStencilView();
	PrepareRenderTargetView();

	const bool isForSelection = false;
	if (isEraseBackground) {
		float aClearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f }; //red,green,blue,alpha
		if (isForSelection) {
			// Default color shall be 0 because D3D_SELECTION_TARGET_NULL is 0.
			// Transparency shall not be used in DRAW_FOR_SELECTION mode.
			aClearColor[3] = 0.0f;
		}
		m_pDc->ClearRenderTargetView(m_pRenderTargetView.Get(), aClearColor);
		m_pDc->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	}

	m_pDc->RSSetViewports(1, &m_viewport);
	m_pDc->RSSetState(m_pRasterizerState.Get());
	m_pDc->OMSetDepthStencilState(m_pDepthStencilState.Get(), 1);

	ID3D11RenderTargetView* apRtv[1] = { m_pRenderTargetView.Get() };
	m_pDc->OMSetRenderTargets(1, apRtv, m_pDepthStencilView.Get());
}

void YmTngnViewModel::EndDraw()
{
	HRESULT hr = m_pSwapChain->Present(0, 0);
	if (FAILED(hr)) {
		YM_IGNORE_ERROR("Present");
	}
}


////////////////////////////////////////////////////////////////////////////////
