#pragma once

#include "YmTngnModelFwd.h"

namespace Ymcpp {

class YmRgba4b
{
public:
	YmRgba4b() : m_rgba(0) {}
	explicit YmRgba4b(int r, int g, int b);
	//virtual ~YmRgba();

	int GetR() const { return m_rgba & 0xFF; }
	int GetG() const { return (m_rgba >> 8) & 0xFF; }
	int GetB() const { return (m_rgba >> 16) & 0xFF; }
	uint32_t ToUint32() const { return m_rgba; }
private:
	uint32_t m_rgba;
};

}
