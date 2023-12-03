#pragma once

#include "YmTngnFwd.h"

namespace Ymcpp {

class YmTngn2DText
{
public:
	YmTngn2DText();
	virtual ~YmTngn2DText();

	YmTngn2DText(const YmTngn2DText& other) = default;
	YmTngn2DText(YmTngn2DText&& other) = default;
	YmTngn2DText& operator = (const YmTngn2DText& other) = default;
	YmTngn2DText& operator = (YmTngn2DText&& other) = default;

public:
	const std::wstring& GetText() const { return m_text; }
	void SetText(const std::wstring& text) { m_text = text; }

	YmVector2i GetOrigin() const { return m_origin; }
	void SetOrigin(const YmVector2i& origin) { m_origin = origin; }

private:
	std::wstring m_text;
	YmVector2i m_origin;
};

}
