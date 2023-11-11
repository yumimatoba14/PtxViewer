#include "pch.h"
#include "YmTngnDmDrawableObjectList.h"
#include "YmTngnDraw.h"
#include <map>

using namespace std;
using namespace Ymcpp;
using namespace DirectX;

////////////////////////////////////////////////////////////////////////////////

YmTngnDmDrawableObjectList::YmTngnDmDrawableObjectList()
{
}

YmTngnDmDrawableObjectList::~YmTngnDmDrawableObjectList()
{
}

////////////////////////////////////////////////////////////////////////////////

void YmTngnDmDrawableObjectList::AddObject(YmTngnModelMatrixPtr pMatrix, YmTngnDrawableObjectPtr pObject)
{
	YM_IS_TRUE(pObject);
	m_objectDataList.push_back(ObjectData{ move(pMatrix), move(pObject) });
}

////////////////////////////////////////////////////////////////////////////////

void YmTngnDmDrawableObjectList::OnDraw(YmTngnDraw* pDraw)
{
	if (false) {
		DrawDirect(pDraw);
	}
	else {
		if (!pDraw->IsProgressiveViewFollowingFrame()) {
			SortObjectsByDepth(pDraw);
		}

		DrawSorted(pDraw);
	}
}

////////////////////////////////////////////////////////////////////////////////

void YmTngnDmDrawableObjectList::DrawDirect(YmTngnDraw* pDraw)
{
	auto modelMatrix = pDraw->MakeModelMatrixSetter();
	for (auto& data : m_objectDataList) {
		modelMatrix.Set(data.m_pModelMatrix);
		YM_ASSERT(data.m_pObject);
		data.m_pObject->Draw(pDraw);
	}
}

void YmTngnDmDrawableObjectList::DrawSorted(YmTngnDraw* pDraw)
{
	auto modelMatrix = pDraw->MakeModelMatrixSetter();
	for (size_t iObject : m_sortedObjectIndexList) {
		auto& data = m_objectDataList.at(iObject);
		modelMatrix.Set(data.m_pModelMatrix);
		YM_ASSERT(data.m_pObject);
		data.m_pObject->Draw(pDraw);
	}
}

static double CalcAabBoxDepth(const XMMATRIX& localToViewMatrix, const YmAabBox3d& aabb)
{
	YM_IS_TRUE(aabb.IsInitialized());
	double depth = DBL_MAX;
	for (int i = 0; i < 8; ++i) {
		float coord[3];
		coord[0] = static_cast<float>((i & 0x01) ? aabb.GetMaxPoint()[0] : aabb.GetMinPoint()[0]);
		coord[1] = static_cast<float>((i & 0x02) ? aabb.GetMaxPoint()[1] : aabb.GetMinPoint()[1]);
		coord[2] = static_cast<float>((i & 0x04) ? aabb.GetMaxPoint()[2] : aabb.GetMinPoint()[2]);

		XMVECTOR localVec = XMVectorSet(coord[0], coord[1], coord[2], 1);
		XMVECTOR viewVec = XMVector4Transform(localVec, localToViewMatrix);
		depth = min(depth, (double)XMVectorGetZ(viewVec));
	}
	return depth;
}

void YmTngnDmDrawableObjectList::SortObjectsByDepth(YmTngnDraw* pDraw)
{
	auto modelMatrix = pDraw->MakeModelMatrixSetter();
	XMMATRIX localToViewMatrix = XMMatrixIdentity();
	size_t nObject = m_objectDataList.size();
	multimap<double, size_t> depthToObjectMap;
	for (size_t iObject = 0; iObject < nObject; ++iObject) {
		auto& data = m_objectDataList[iObject];
		if (modelMatrix.Set(data.m_pModelMatrix)) {
			localToViewMatrix = pDraw->GetModelToViewMatrix();
		}

		double depth = CalcAabBoxDepth(localToViewMatrix, data.m_pObject->GetAabBox());
		if (0 < depth) {
			continue;
		}
		depthToObjectMap.insert(make_pair(depth, iObject));
	}

	m_sortedObjectIndexList.clear();
	for (auto& pair : depthToObjectMap) {
		m_sortedObjectIndexList.push_back(pair.second);
	}
}

////////////////////////////////////////////////////////////////////////////////
