#include "pch.h"
#include "YmTextDrawerImpl.h"
#include <atlbase.h>
#include <atlconv.h>
#include <vector>

using namespace std;
using namespace Ymcpp;

////////////////////////////////////////////////////////////////////////////////

static D2D1_COLOR_F ToColorF(const YmRgba4b in) {
	// D2D1_COLOR_F is BGRA. So ToUInt32() is not available.
	D2D1_COLOR_F bgra;
	bgra.r = in.GetR() / 255.0f;
	bgra.g = in.GetG() / 255.0f;
	bgra.b = in.GetB() / 255.0f;
	bgra.a = in.GetA() / 255.0f;
	return bgra;
}

YmTextDrawerImpl::YmTextDrawerImpl(const DXGISurfacePtr& pSurface)
{
	RecreateRenderTarget(pSurface);

	HRESULT hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), &m_pDWriteFactory);
	if (FAILED(hr)) {
		YM_THROW_ERROR("DWriteCreateFactory");
	}

	YmTString fontName = _T("ƒƒCƒŠƒI");
	YmTString localeName = _T("");
	FLOAT fontSize = 20;
	hr = m_pDWriteFactory->CreateTextFormat(
		CT2W(fontName.c_str()), nullptr,
		DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
		fontSize, CT2W(localeName.c_str()),
		&m_pDefaultTextFormat);
	if (FAILED(hr)) {
		YM_THROW_ERROR("CreateTextFormat");
	}

	hr = m_pDefaultTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
	if (FAILED(hr)) {
		YM_THROW_ERROR("SetTextAlignment");
	}

	PrepareTextBrush();
	PrepareBackgroundBrush();
}

YmTextDrawerImpl::~YmTextDrawerImpl()
{
}

////////////////////////////////////////////////////////////////////////////////

void YmTextDrawerImpl::DrawTextWithoutBackground(LPCWSTR pText, const YmVector2i& pos)
{
	D2D1_RECT_F rect = D2D1::RectF((float)pos[0], (float)pos[1] - 20, FLT_MAX, FLT_MAX);
	m_pRenderTarget->DrawText(pText, static_cast<UINT32>(wcslen(pText)), m_pDefaultTextFormat.Get(),
		rect, PrepareTextBrush(), D2D1_DRAW_TEXT_OPTIONS_NONE);
}

void YmTextDrawerImpl::DrawText(LPCWSTR pText, const YmVector2i& pos)
{
	DWriteTextLayoutPtr pTextLayout;
	HRESULT hr = m_pDWriteFactory->CreateTextLayout(pText, static_cast<UINT32>(wcslen(pText)), m_pDefaultTextFormat.Get(),
		FLT_MAX, FLT_MAX, &pTextLayout);
	if (FAILED(hr)) {
		YM_THROW_ERROR("CreateTextLayout");
	}

	DWRITE_TEXT_METRICS metrics;
	hr = pTextLayout->GetMetrics(&metrics);
	if (FAILED(hr)) {
		YM_THROW_ERROR("GetMetrics");
	}
	if (metrics.lineCount < 1) {
		return;
	}

	vector<DWRITE_LINE_METRICS> aLineMetrics(metrics.lineCount);
	UINT32 numLine = 0;
	hr = pTextLayout->GetLineMetrics(aLineMetrics.data(), static_cast<UINT32>(aLineMetrics.size()), &numLine);
	if (FAILED(hr)) {
		YM_THROW_ERROR("CreateTextLayout");
	}
	YM_ASSERT(numLine == metrics.lineCount);

	D2D1_POINT_2F origin = D2D1::Point2F(
		(float)pos[0] + metrics.left,
		(float)pos[1] - aLineMetrics[0].baseline + metrics.top);
	D2D1_RECT_F rect = D2D1::RectF(origin.x, origin.y,
		origin.x + metrics.left + metrics.width, origin.y + metrics.top + metrics.height);

	if (m_backgroundColor.GetA() != 0) {
		m_pRenderTarget->FillRectangle(rect, PrepareBackgroundBrush());
	}
	m_pRenderTarget->DrawTextLayout(
		origin, pTextLayout.Get(), PrepareTextBrush(), D2D1_DRAW_TEXT_OPTIONS_NONE);
}

////////////////////////////////////////////////////////////////////////////////

void YmTextDrawerImpl::OnResizeBuffer()
{
	m_pRenderTarget = nullptr;
	m_pTextBrush = nullptr;
	m_pBackgroundBrush = nullptr;
}

void YmTextDrawerImpl::RecreateRenderTarget(const DXGISurfacePtr& pSurface)
{
	m_pRenderTarget = nullptr;

	D2D1FactoryPtr pD2dFactory;
	HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, IID_PPV_ARGS(&pD2dFactory));
	if (FAILED(hr)) {
		YM_THROW_ERROR("D2D1CreateFactory");
	}

	// TODO: GetDpiForWindows(HWND) function is recommended.
	FLOAT dpiX, dpiY;
	pD2dFactory->GetDesktopDpi(&dpiX, &dpiY);

	D2D1_RENDER_TARGET_PROPERTIES rtProperties = D2D1::RenderTargetProperties(
		D2D1_RENDER_TARGET_TYPE_DEFAULT,
		D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED),
		dpiX, dpiY);

	hr = pD2dFactory->CreateDxgiSurfaceRenderTarget(pSurface.Get(), &rtProperties, &m_pRenderTarget);
	if (FAILED(hr)) {
		YM_THROW_ERROR("CreateDxgiSurfaceRenderTarget");
	}
}

ID2D1Brush* YmTextDrawerImpl::PrepareTextBrush()
{
	YM_IS_TRUE(m_pRenderTarget != nullptr);
	if (!m_pTextBrush) {
		D2D1SolidColorBrushPtr pTextSolidColorBrush;
		HRESULT hr = m_pRenderTarget->CreateSolidColorBrush(ToColorF(m_textColor), &pTextSolidColorBrush);
		if (FAILED(hr)) {
			YM_THROW_ERROR("CreateSolidColorBrush");
		}
		m_pTextBrush = pTextSolidColorBrush;
	}
	return m_pTextBrush.Get();
}

ID2D1Brush* YmTextDrawerImpl::PrepareBackgroundBrush()
{
	YM_IS_TRUE(m_pRenderTarget != nullptr);
	if (!m_pBackgroundBrush) {
		D2D1SolidColorBrushPtr pBgSolidColorBrush;
		HRESULT hr = m_pRenderTarget->CreateSolidColorBrush(ToColorF(m_backgroundColor), &pBgSolidColorBrush);
		if (FAILED(hr)) {
			YM_THROW_ERROR("CreateSolidColorBrush");
		}
		m_pBackgroundBrush = pBgSolidColorBrush;
	}
	return m_pBackgroundBrush.Get();
}

////////////////////////////////////////////////////////////////////////////////
