#pragma once

#include "YmTngnModel.h"
#include <vector>

namespace Ymcpp {

class YmTngnIndexedTriangleList
{
public:
	using VertexIndex = uint32_t;
	struct VertexType {
		YmVector3f position;
		YmVector3f normalDir;
	};
	using TriangleType = YmVectorN<3, VertexIndex>;
public:
	YmTngnIndexedTriangleList();
	virtual ~YmTngnIndexedTriangleList();

	YmTngnIndexedTriangleList(const YmTngnIndexedTriangleList& other) = delete;
	YmTngnIndexedTriangleList& operator = (const YmTngnIndexedTriangleList& other) = delete;

	size_t GetVertexCount() const { return m_vertices.size(); }
	const VertexType& GetVertexAt(size_t i) const { return m_vertices.at(i); }
	VertexIndex AddVertex(const VertexType& vtx) { m_vertices.push_back(vtx); return static_cast<VertexIndex>(m_vertices.size() - 1); }
	void ClearVertex() { m_vertices.clear(); }
	void ReserveVertex(size_t capacity) { m_vertices.reserve(capacity); }

	size_t GetTriangleCount() const { return m_triangles.size(); }
	const TriangleType& GetTriangleVertexAt(size_t i) const { return m_triangles.at(i); }
	size_t AddTriangle(const TriangleType& tri);
	size_t AddTriangle(VertexIndex v0, VertexIndex v1, VertexIndex v2) { return AddTriangle(YmVectorUtil::Make(v0, v1, v2)); }
	void ClearTriangle() { m_triangles.clear(); }
	void ReserveTriangle(size_t capacity) { m_triangles.reserve(capacity); }

	YmRgba4b GetColor() const { return m_color; }
	void SetColor(const YmRgba4b& rgba) { m_color = rgba; }
	bool IsTransparent() const { return m_color.GetA() != 255; }
private:
	std::vector<VertexType> m_vertices;
	std::vector<TriangleType> m_triangles;
	YmRgba4b m_color;
};

}
