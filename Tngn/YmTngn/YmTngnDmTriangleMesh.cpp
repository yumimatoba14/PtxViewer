#include "pch.h"
#include "YmTngnDmTriangleMesh.h"
#include "YmTngnDraw.h"
#include "YmTngnModel/YmTngnIndexedTriangleList.h"

using namespace std;
using namespace Ymcpp;
using namespace DirectX;

////////////////////////////////////////////////////////////////////////////////

YmTngnDmTriangleMesh::YmTngnDmTriangleMesh()
	: m_pLocalToGlobalMatrix(make_shared<XMFLOAT4X4>())
{
	XMStoreFloat4x4(m_pLocalToGlobalMatrix.get(), XMMatrixIdentity());
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

void YmTngnDmTriangleMesh::SetColor(YmRgba4b color)
{
	for (auto& pList : m_indexedTriangleLists) {
		pList->SetColor(color);
	}
}

std::vector<YmTngnDmTriangleMesh::IndexedTriangleListPtr>
YmTngnDmTriangleMesh::FindPickedTriangles(YmTngnPickTargetId id) const
{
	if (IsPickEnabled() && id != YM_TNGN_PICK_TARGET_NULL) {
		vector<IndexedTriangleListPtr> results;
		for (auto& pList : m_indexedTriangleLists) {
			if (pList->GetPickTargetId() == id) {
				results.push_back(pList);
			}
		}
		return results;
	}
	return vector<IndexedTriangleListPtr>();
}

YmTngnDmTriangleMeshPtr YmTngnDmTriangleMesh::CreateClone() const
{
	YmTngnDmTriangleMeshPtr pResult = make_shared< YmTngnDmTriangleMesh>();
	pResult->SetLocalToGlobalMatrix(GetLocalToGlobalMatrix());
	for (auto pTri : m_indexedTriangleLists) {
		pResult->m_indexedTriangleLists.push_back(pTri->CreateClone());
	}
	return pResult;
}

////////////////////////////////////////////////////////////////////////////////

YmTngnDmTriangleMeshPtr YmTngnDmTriangleMesh::MakeSampleData(YmVector3d origin)
{
	auto pMesh = make_shared<YmTngnDmTriangleMesh>();
	pMesh->m_pLocalToGlobalMatrix->m[3][0] = static_cast<float>(origin[0]);
	pMesh->m_pLocalToGlobalMatrix->m[3][1] = static_cast<float>(origin[1]);
	pMesh->m_pLocalToGlobalMatrix->m[3][2] = static_cast<float>(origin[2]);

	double y = -1;
	YmTngnIndexedTriangleList::VertexType vertex;

	YmTngnIndexedTriangleListPtr pTestData = make_shared<YmTngnIndexedTriangleList>();
	vertex.normalDir = YmVectorUtil::StaticCast<XMFLOAT3>(YmVectorUtil::Make(0, 1, 0));
	vertex.position = YmVectorUtil::StaticCast<XMFLOAT3>(YmVectorUtil::Make<double>(-1, y, 0));
	pTestData->AddVertex(vertex);
	vertex.position = YmVectorUtil::StaticCast<XMFLOAT3>(YmVectorUtil::Make<double>(1, y, 0));
	pTestData->AddVertex(vertex);
	vertex.position = YmVectorUtil::StaticCast<XMFLOAT3>(YmVectorUtil::Make<double>(1, y, 1));
	pTestData->AddVertex(vertex);
	vertex.position = YmVectorUtil::StaticCast<XMFLOAT3>(YmVectorUtil::Make<double>(-1, y, 1));
	pTestData->AddVertex(vertex);
	pTestData->SetColor(YmRgba4b(255, 255, 0));
	pTestData->AddTriangle(0, 1, 2);
	pTestData->AddTriangle(2, 3, 0);
	pMesh->AddIndexedTriangleList(pTestData);

	y = 0;
	pTestData = make_shared<YmTngnIndexedTriangleList>();
	vertex.position = YmVectorUtil::StaticCast<XMFLOAT3>(YmVectorUtil::Make<double>(-1, y, 0));
	pTestData->AddVertex(vertex);
	vertex.position = YmVectorUtil::StaticCast<XMFLOAT3>(YmVectorUtil::Make<double>(1, y, 0));
	pTestData->AddVertex(vertex);
	vertex.position = YmVectorUtil::StaticCast<XMFLOAT3>(YmVectorUtil::Make<double>(1, y, 1));
	pTestData->AddVertex(vertex);
	vertex.position = YmVectorUtil::StaticCast<XMFLOAT3>(YmVectorUtil::Make<double>(-1, y, 1));
	pTestData->AddVertex(vertex);
	pTestData->SetColor(YmRgba4b(0, 255, 255, 128));
	pTestData->AddTriangle(0, 1, 2);
	pTestData->AddTriangle(2, 3, 0);
	pMesh->AddIndexedTriangleList(pTestData);

	y = 0.5;
	pTestData = make_shared<YmTngnIndexedTriangleList>();
	vertex.position = YmVectorUtil::StaticCast<XMFLOAT3>(YmVectorUtil::Make<double>(-1, y, 0));
	pTestData->AddVertex(vertex);
	vertex.position = YmVectorUtil::StaticCast<XMFLOAT3>(YmVectorUtil::Make<double>(0, y, 0));
	pTestData->AddVertex(vertex);
	vertex.position = YmVectorUtil::StaticCast<XMFLOAT3>(YmVectorUtil::Make<double>(0, y, 2));
	pTestData->AddVertex(vertex);
	vertex.position = YmVectorUtil::StaticCast<XMFLOAT3>(YmVectorUtil::Make<double>(-1, y, 2));
	pTestData->AddVertex(vertex);
	pTestData->SetColor(YmRgba4b(255, 255, 128, 128));
	pTestData->AddTriangle(0, 1, 2);
	pTestData->AddTriangle(2, 3, 0);
	pMesh->AddIndexedTriangleList(pTestData);

	y = 1;
	pTestData = make_shared<YmTngnIndexedTriangleList>();
	vertex.position = YmVectorUtil::StaticCast<XMFLOAT3>(YmVectorUtil::Make<double>(-1, y, 0));
	pTestData->AddVertex(vertex);
	vertex.position = YmVectorUtil::StaticCast<XMFLOAT3>(YmVectorUtil::Make<double>(1, y, 0));
	pTestData->AddVertex(vertex);
	vertex.position = YmVectorUtil::StaticCast<XMFLOAT3>(YmVectorUtil::Make<double>(1, y, 1));
	pTestData->AddVertex(vertex);
	vertex.position = YmVectorUtil::StaticCast<XMFLOAT3>(YmVectorUtil::Make<double>(-1, y, 1));
	pTestData->AddVertex(vertex);
	pTestData->SetColor(YmRgba4b(255, 0, 255));
	pTestData->AddTriangle(0, 1, 2);
	pTestData->AddTriangle(2, 3, 0);
	pMesh->AddIndexedTriangleList(pTestData);

	return pMesh;
}

static YmVector3d MakeSeedDir(const YmVector3d& vecN)
{
	int minI = 0;
	for (int i = 1; i < 3; ++i) {
		if (fabs(vecN[i]) < fabs(vecN[minI])) {
			minI = i;
		}
	}
	YmVector3d seed = YmVector3d::MakeZero();
	seed[minI] = 1;
	return seed;
}

static YmTngnDmTriangleMeshPtr MakeExtrudedSideFaceMesh(
	double zeroTol, const vector<YmVector3d>& profilePoints, const YmVector3d& vecN, double height, YmRgba4b color
)
{
	auto pMesh = make_shared<YmTngnDmTriangleMesh>();
	YmTngnIndexedTriangleList::VertexType vertex;
	size_t nProfilePoint = profilePoints.size();
	for (size_t iProfilePoint = 1; iProfilePoint < nProfilePoint; ++iProfilePoint) {
		YmVector3d edgeDir = profilePoints[iProfilePoint] - profilePoints[iProfilePoint - 1];
		bool isOk = YmVectorUtil::TryNormalize(zeroTol, edgeDir, &edgeDir);
		if (!isOk) {
			continue;
		}
		vertex.normalDir = YmVectorUtil::StaticCast<YmVector3f>(YmVectorUtil::OuterProduct(edgeDir, vecN));
		vertex.normalDir = YmVectorUtil::Normalize(zeroTol, vertex.normalDir);

		YmTngnIndexedTriangleListPtr pPlane = make_shared<YmTngnIndexedTriangleList>();
		vertex.position = YmVectorUtil::StaticCast<XMFLOAT3>(profilePoints[iProfilePoint - 1]);
		size_t index = pPlane->AddVertex(vertex);
		YM_ASSERT(index == 0);
		vertex.position = YmVectorUtil::StaticCast<XMFLOAT3>(profilePoints[iProfilePoint]);
		pPlane->AddVertex(vertex);
		vertex.position = YmVectorUtil::StaticCast<XMFLOAT3>(profilePoints[iProfilePoint - 1] + vecN * height);
		pPlane->AddVertex(vertex);
		vertex.position = YmVectorUtil::StaticCast<XMFLOAT3>(profilePoints[iProfilePoint] + vecN * height);
		pPlane->AddVertex(vertex);
		pPlane->AddTriangle(0, 1, 2);
		pPlane->AddTriangle(3, 2, 1);
		pPlane->SetColor(color);
		pMesh->AddIndexedTriangleList(pPlane);
	}

	return pMesh;
}

static YmTngnIndexedTriangleListPtr MakeExtrudedTangentSideFace(
	double zeroTol, const vector<YmVector3d>& profilePoints, const vector<YmVector3d>& profileNormal, const YmVector3d& offset, YmRgba4b color
)
{
	YM_IS_TRUE(profilePoints.size() == profileNormal.size());
	YmTngnIndexedTriangleListPtr pTriangleList = make_shared<YmTngnIndexedTriangleList>();

	YmTngnIndexedTriangleList::VertexType vertex;
	YM_IS_TRUE(profilePoints.size() <= MAXUINT32);
	uint32_t nProfilePoint = static_cast<uint32_t>(profilePoints.size());
	for (uint32_t iProfilePoint = 0; iProfilePoint < nProfilePoint; ++iProfilePoint) {
		vertex.position = YmVectorUtil::StaticCast<XMFLOAT3>(profilePoints[iProfilePoint]);
		vertex.normalDir = YmVectorUtil::StaticCast<XMFLOAT3>(
			YmVectorUtil::Normalize(zeroTol, profileNormal[iProfilePoint]));

		size_t index = pTriangleList->AddVertex(vertex);
		YM_ASSERT(index == iProfilePoint * 2);

		vertex.position = YmVectorUtil::StaticCast<XMFLOAT3>(profilePoints[iProfilePoint] + offset);
		index = pTriangleList->AddVertex(vertex);
		YM_ASSERT(index == iProfilePoint * 2 + 1);
	}

	for (uint32_t iProfilePoint = 0; iProfilePoint < nProfilePoint; ++iProfilePoint) {
		uint32_t index0 = (iProfilePoint == 0 ? nProfilePoint - 1 : iProfilePoint - 1) * 2;
		uint32_t index1 = iProfilePoint * 2;
		pTriangleList->AddTriangle(index0, index1, index0 + 1);
		pTriangleList->AddTriangle(index1 + 1, index0 + 1, index1);
	}
	
	pTriangleList->SetColor(color);
	return pTriangleList;
}

std::shared_ptr<YmTngnDmTriangleMesh>
YmTngnDmTriangleMesh::MakeSampleCylinderData(
	double approxTol, YmVector3d origin, YmVector3d axisDir, double radius, double height, YmRgba4b color, bool smooth
)
{
	const double zeroTol = 1e-8;
	YM_IS_TRUE(zeroTol < approxTol);
	YM_IS_TRUE(zeroTol < radius);

	YmVector3d vecN = YmVectorUtil::Normalize(zeroTol, axisDir);
	YmVector3d vecU = MakeSeedDir(vecN);
	YmVector3d vecV = YmVectorUtil::Normalize(zeroTol, YmVectorUtil::OuterProduct(vecN, vecU));
	vecU = YmVectorUtil::OuterProduct(vecV, vecN);

	// error = radius - radius * cos(theta/2)
	// ->
	// theta = 2*acos((radius - error) / radius)
	double theta = 2 * acos((radius - approxTol) / radius);
	int numDiv = static_cast<int>(ceil(2 * M_PI / theta));
	numDiv = max(4, numDiv);

	vector<YmVector3d> loopPoints, loopNormal;
	for (int i = 0; i < numDiv; ++i) {
		double theta = 2 * M_PI / numDiv * i;
		YmVector3d radiusDir = cos(theta) * vecU + sin(theta) * vecV;
		YmVector3d point = origin + radius * radiusDir;
		loopPoints.push_back(point);
		if (smooth) {
			loopNormal.push_back(radiusDir);
		}
	}
	if (!smooth) {
		loopPoints.push_back(loopPoints.front());
	}

	if (smooth) {
		auto pMesh = make_shared<YmTngnDmTriangleMesh>();
		pMesh->AddIndexedTriangleList(
			MakeExtrudedTangentSideFace(zeroTol, loopPoints, loopNormal, vecN * height, color)
		);
		return pMesh;
	}
	else {
		return MakeExtrudedSideFaceMesh(zeroTol, loopPoints, vecN, height, color);
	}
}

////////////////////////////////////////////////////////////////////////////////

bool YmTngnDmTriangleMesh::OnSetPickEnabled(bool isEnable)
{
	return isEnable;
}

void YmTngnDmTriangleMesh::OnDraw(YmTngnDraw* pDraw)
{
	if (pDraw->IsProgressiveViewFollowingFrame()) {
		return;
	}
	bool isPickMode = IsPickEnabled();
	YmTngnPickTargetId nextPickId = YM_TNGN_PICK_TARGET_NULL;
	if (isPickMode) {
		nextPickId = pDraw->MakePickTargetId(m_indexedTriangleLists.size());
	}

	pDraw->SetModelMatrix(GetLocalToGlobalMatrix());
	for (auto pObj : m_indexedTriangleLists) {
		if (isPickMode) {
			pObj->SetPickTargetId(nextPickId++);
		}
		else {
			pObj->SetPickTargetId(YM_TNGN_PICK_TARGET_NULL);
		}
		if (pObj->IsTransparent()) {
			pObj->PrepareAabb();
			pDraw->RegisterTransparentObject(m_pLocalToGlobalMatrix, pObj);
		}
		else {
			pObj->Draw(pDraw);
		}
	}
	pDraw->ClearModelMatrix();
}

////////////////////////////////////////////////////////////////////////////////

YmTngnDmTriangleMesh::IndexedTriangleList::IndexedTriangleList(YmTngnIndexedTriangleListPtr pModel)
	: m_pModel(std::move(pModel))
{
	if (m_pModel) {
		m_color = m_pModel->GetColor();
	}
}

void YmTngnDmTriangleMesh::IndexedTriangleList::Draw(YmTngnDraw* pDraw)
{
	YM_IS_TRUE(pDraw != nullptr);
	if (!m_pVertexBuffer || !m_pIndexBuffer) {
		PrepareData(pDraw);
	}
	if (m_pVertexBuffer && m_pIndexBuffer) {
		pDraw->DrawTriangleList(m_pVertexBuffer, m_pIndexBuffer, m_nIndex, m_pickTargetId);
	}
}

void YmTngnDmTriangleMesh::IndexedTriangleList::PrepareAabb()
{
	if (GetAabBox().IsInitialized()) {
		return;
	}

	YmAabBox3d aabb;
	size_t nVertex = m_pModel->GetVertexCount();
	for (size_t i = 0; i < nVertex; ++i) {
		auto inVtx = m_pModel->GetVertexAt(i);
		aabb.Extend(inVtx.position);
	}
	SetAabBox(aabb);
}

YmTngnDmTriangleMesh::IndexedTriangleListPtr YmTngnDmTriangleMesh::IndexedTriangleList::CreateClone() const
{
	auto pTri = make_shared<IndexedTriangleList>(m_pModel);
	pTri->SetColor(GetColor());
	return pTri;
}

void YmTngnDmTriangleMesh::IndexedTriangleList::PrepareData(YmTngnDraw* pDraw)
{
	ClearData();
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
		vtx.rgba = m_color.ToUint32();
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
	m_pIndexBuffer = pDraw->CreateIndexBuffer(indices.data(), (UINT)indices.size());
	m_nIndex = indices.size();
}

////////////////////////////////////////////////////////////////////////////////
