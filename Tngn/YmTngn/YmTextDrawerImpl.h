#pragma once

#include "YmTngnFwd.h"
#include "YmTngnModel/YmRgba.h"
#include <dwrite.h>

namespace Ymcpp {

using DWriteFactoryPtr = YmComPtr<IDWriteFactory>;
using DWriteTextFormatPtr = YmComPtr<IDWriteTextFormat>;

class YmTextDrawerImpl
{
public:
	explicit YmTextDrawerImpl(const DXGISurfacePtr& pSurface);
	virtual ~YmTextDrawerImpl();

	YmTextDrawerImpl(const YmTextDrawerImpl& other) = delete;
	YmTextDrawerImpl& operator = (const YmTextDrawerImpl& other) = delete;

public:
	YmRgba4b GetTextColor() const { return m_textColor; }
	void SetTextColor(const YmRgba4b& color) { m_textColor = color; m_pTextBrush = nullptr; }

	YmRgba4b GetBackgroundColor() const { return m_backgroundColor; }
	void SetBackgroundColor(const YmRgba4b& color) { m_backgroundColor = color; m_pBackgroundBrush = nullptr; }

	void BeginDraw() { m_pRenderTarget->BeginDraw(); }
	void EndDraw() { m_pRenderTarget->EndDraw(); }

	void DrawTextWithoutBackground(LPCWSTR pText, const YmVector2i& pos);
	void DrawText(LPCWSTR pText, const YmVector2i& pos);

public:
	void OnResizeBuffer();
	void RecreateRenderTarget(const DXGISurfacePtr& pSurface);

private:
	ID2D1Brush* PrepareTextBrush();
	ID2D1Brush* PrepareBackgroundBrush();

private:
	DWriteFactoryPtr m_pDWriteFactory;
	DWriteTextFormatPtr m_pDefaultTextFormat;
	YmRgba4b m_textColor = YmRgba4b(255, 255, 255);
	YmRgba4b m_backgroundColor = YmRgba4b(96, 96, 96, 192);

	// work object to be recreated.
private:
	D2D1RenderTargetPtr m_pRenderTarget;
	D2D1BrushPtr m_pTextBrush;
	D2D1BrushPtr m_pBackgroundBrush;
};

using YmTextDrawerImplPtr = std::shared_ptr<YmTextDrawerImpl>;

}

#pragma comment(lib, "dwrite.lib")
