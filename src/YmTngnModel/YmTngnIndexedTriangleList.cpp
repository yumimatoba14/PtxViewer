#include "pch.h"
#include "YmTngnIndexedTriangleList.h"

using namespace std;
using namespace Ymcpp;

////////////////////////////////////////////////////////////////////////////////

YmTngnIndexedTriangleList::YmTngnIndexedTriangleList()
{
}

YmTngnIndexedTriangleList::~YmTngnIndexedTriangleList()
{
}

////////////////////////////////////////////////////////////////////////////////

size_t YmTngnIndexedTriangleList::AddTriangle(const TriangleType& tri)
{
	YM_IS_TRUE(tri[0] < GetVertexCount());
	YM_IS_TRUE(tri[1] < GetVertexCount());
	YM_IS_TRUE(tri[2] < GetVertexCount());
	m_triangles.push_back(tri);
	return m_triangles.size() - 1;
}

////////////////////////////////////////////////////////////////////////////////
