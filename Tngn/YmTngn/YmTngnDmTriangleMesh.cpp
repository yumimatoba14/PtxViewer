#include "pch.h"
#include "YmTngnDmTriangleMesh.h"
#include "YmTngnDraw.h"
#include "YmTngnModel/YmTngnIndexedTriangleList.h"

using namespace std;
using namespace Ymcpp;
using namespace DirectX;

////////////////////////////////////////////////////////////////////////////////

YmTngnDmTriangleMesh::YmTngnDmTriangleMesh()
{
	XMStoreFloat4x4(&m_localToGlobalMatrix, XMMatrixIdentity());
}

YmTngnDmTriangleMesh::~YmTngnDmTriangleMesh()
{
}

////////////////////////////////////////////////////////////////////////////////

void YmTngnDmTriangleMesh::AddIndexedTriangleList(const YmTngnIndexedTriangleListPtr& pTriList)
{
	YM_IS_TRUE(pTriList);
	m_indexedTriangleLists.push_back(make_shared<IndexedTriangleList>(pTriList));
}

////////////////////////////////////////////////////////////////////////////////

void YmTngnDmTriangleMesh::OnDraw(YmTngnDraw* pDraw)
{
	PrepareTestData();

	pDraw->SetModelMatrix(m_localToGlobalMatrix);
	for (auto pObj : m_indexedTriangleLists) {
		pObj->Draw(pDraw);
	}
	pDraw->ClearModelMatrix();
}

////////////////////////////////////////////////////////////////////////////////

void YmTngnDmTriangleMesh::IndexedTriangleList::Draw(YmTngnDraw* pDraw)
{
	YM_IS_TRUE(pDraw != nullptr);
	if (!m_pVertexBuffer || !m_pIndexBuffer) {
		PrepareData(pDraw);
	}
	if (m_pVertexBuffer && m_pIndexBuffer) {
		pDraw->DrawTriangleList(m_pVertexBuffer, m_pIndexBuffer, m_nIndex);
	}
}

void YmTngnDmTriangleMesh::IndexedTriangleList::PrepareData(YmTngnDraw* pDraw)
{
	m_pVertexBuffer = nullptr;
	m_pIndexBuffer = nullptr;
	m_nIndex = 0;
	if (!m_pModel) {
		return;
	}

	size_t nVertex = m_pModel->GetVertexCount();
	size_t nTriangle = m_pModel->GetTriangleCount();
	vector<VertexType> vertices;
	vertices.reserve(nVertex);
	for (size_t i = 0; i < nVertex; ++i) {
		auto inVtx = m_pModel->GetVertexAt(i);
		VertexType vtx;
		vtx.position = YmVectorUtil::StaticCast<XMFLOAT3>(inVtx.position);
		vtx.normalDir = YmVectorUtil::StaticCast<XMFLOAT3>(inVtx.normalDir);
		vtx.rgba = m_pModel->GetColor().ToUint32();
		vertices.push_back(vtx);
	}
	
	vector<IndexType> indices;
	indices.reserve(nTriangle * 3);
	for (size_t i = 0; i < nTriangle; ++i) {
		auto inTri = m_pModel->GetTriangleVertexAt(i);
		indices.push_back(inTri[0]);
		indices.push_back(inTri[1]);
		indices.push_back(inTri[2]);
	}

	m_pVertexBuffer = pDraw->CreateVertexBuffer(vertices.data(), (UINT)vertices.size(), false);
	m_pIndexBuffer = pDraw->CreateInexBuffer(indices.data(), (UINT)indices.size());
	m_nIndex = indices.size();
}

////////////////////////////////////////////////////////////////////////////////
