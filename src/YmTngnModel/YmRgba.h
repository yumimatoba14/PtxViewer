#pragma once

#include "YmTngnModelFwd.h"

namespace Ymcpp {

class YmRgba4b
{
public:
	YmRgba4b() : m_rgba(0) {}
	explicit YmRgba4b(int r, int g, int b, int a = 255);
	//virtual ~YmRgba();

	int GetR() const { return m_rgba & 0xFF; }
	int GetG() const { return (m_rgba >> 8) & 0xFF; }
	int GetB() const { return (m_rgba >> 16) & 0xFF; }
	int GetA() const { return (m_rgba >> 24) & 0xFF; }
	uint32_t ToUint32() const { return m_rgba; }

	YmRgba4b RemoveAlpha() const
	{
		return YmRgba4b(GetR(), GetG(), GetB());
	}
private:
	uint32_t m_rgba;
};

inline bool operator == (const YmRgba4b& lhs, const YmRgba4b& rhs) { return lhs.ToUint32() == rhs.ToUint32(); }
inline bool operator != (const YmRgba4b& lhs, const YmRgba4b& rhs) { return lhs.ToUint32() != rhs.ToUint32(); }
inline bool operator < (const YmRgba4b& lhs, const YmRgba4b& rhs) { return lhs.ToUint32() < rhs.ToUint32(); }
inline bool operator > (const YmRgba4b& lhs, const YmRgba4b& rhs) { return lhs.ToUint32() > rhs.ToUint32(); }

}
