#pragma once

#include "YmTngnModelFwd.h"

namespace Ymcpp {

class YmObjToIndexedTriangleListConverter
{
public:
	YmObjToIndexedTriangleListConverter();
	virtual ~YmObjToIndexedTriangleListConverter();

	YmObjToIndexedTriangleListConverter(const YmObjToIndexedTriangleListConverter& other) = delete;
	YmObjToIndexedTriangleListConverter(YmObjToIndexedTriangleListConverter&& other) noexcept = delete;

	YmObjToIndexedTriangleListConverter& operator = (const YmObjToIndexedTriangleListConverter& other) = delete;
	YmObjToIndexedTriangleListConverter& operator = (YmObjToIndexedTriangleListConverter&& other) noexcept = delete;
public:
	YmRgba4b GetDefaultMaterialColor() const { return m_defaultMaterialColor; }
	void SetDefaultMaterialColor(const YmRgba4b& rgba) { m_defaultMaterialColor = rgba; }

	std::vector<YmTngnIndexedTriangleListPtr> Convert(const char* pFilePath);

public:
	static YmRgba4b ConvertMaterialColorToRgb(const YmRgba4b& ambient, const YmRgba4b& diffuse, const YmRgba4b& specular);

private:
	YmRgba4b m_defaultMaterialColor = YmRgba4b(192, 192, 192);
};

}
