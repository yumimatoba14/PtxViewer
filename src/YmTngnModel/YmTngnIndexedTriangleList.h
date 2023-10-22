#pragma once

#include "YmTngnModel.h"

namespace Ymcpp {

class YmTngnIndexedTriangleList
{
public:
	struct VertexType {
		YmVector3f position;
		YmVector3f normalDir;
	};
	using TriangleType = YmVectorN<3, uint32_t>;
public:
	YmTngnIndexedTriangleList();
	virtual ~YmTngnIndexedTriangleList();

	YmTngnIndexedTriangleList(const YmTngnIndexedTriangleList& other) = delete;
	YmTngnIndexedTriangleList& operator = (const YmTngnIndexedTriangleList& other) = delete;

	size_t GetVertexCount() const { return m_vertices.size(); }
	const VertexType& GetVertexAt(size_t i) const { return m_vertices.at(i); }
	size_t AddVertex(const VertexType& vtx) { m_vertices.push_back(vtx); return m_vertices.size() - 1; }
	void ClearVertex() { m_vertices.clear(); }
	void ReserveVertex(size_t capacity) { m_vertices.reserve(capacity); }

	size_t GetTriangleCount() const { return m_triangles.size(); }
	const TriangleType& GetTriangleVertexAt(size_t i) const { return m_triangles.at(i); }
	size_t AddTriangle(const TriangleType& tri);
	size_t AddTriangle(uint32_t v0, uint32_t v1, uint32_t v2) { return AddTriangle(YmVectorUtil::Make(v0, v1, v2)); }
	void ClearTriangle() { m_triangles.clear(); }
	void ReserveTriangle(size_t capacity) { m_triangles.reserve(capacity); }

	YmRgba4b GetColor() const { return m_color; }
	void SetColor(const YmRgba4b& rgba) { m_color = rgba; }
private:
	std::vector<VertexType> m_vertices;
	std::vector<TriangleType> m_triangles;
	YmRgba4b m_color;
};

}
