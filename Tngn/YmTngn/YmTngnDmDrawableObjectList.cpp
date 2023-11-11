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
	XMFLOAT4X4* pLastMatrix = nullptr;
	pDraw->ClearModelMatrix();
	for (auto& data : m_objectDataList) {
		if (pLastMatrix != data.m_pModelMatrix.get()) {
			pLastMatrix = data.m_pModelMatrix.get();
			if (pLastMatrix == nullptr) {
				pDraw->ClearModelMatrix();
			}
			else {
				pDraw->SetModelMatrix(*pLastMatrix);
			}
		}
		YM_ASSERT(data.m_pObject);
		data.m_pObject->Draw(pDraw);
	}
	if (pLastMatrix) {
		pDraw->ClearModelMatrix();
	}
}

void YmTngnDmDrawableObjectList::DrawSorted(YmTngnDraw* pDraw)
{
	XMFLOAT4X4* pLastMatrix = nullptr;
	for (size_t iObject : m_sortedObjectIndexList) {
		auto& data = m_objectDataList.at(iObject);
		if (pLastMatrix != data.m_pModelMatrix.get()) {
			pLastMatrix = data.m_pModelMatrix.get();
			if (pLastMatrix == nullptr) {
				pDraw->ClearModelMatrix();
			}
			else {
				pDraw->SetModelMatrix(*pLastMatrix);
			}
		}
		YM_ASSERT(data.m_pObject);
		data.m_pObject->Draw(pDraw);
	}
	if (pLastMatrix) {
		pDraw->ClearModelMatrix();
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
	XMFLOAT4X4* pLastMatrix = nullptr;
	XMMATRIX localToViewMatrix = XMMatrixIdentity();
	size_t nObject = m_objectDataList.size();
	multimap<double, size_t> depthToObjectMap;
	for (size_t iObject = 0; iObject < nObject; ++iObject) {
		auto& data = m_objectDataList[iObject];
		if (pLastMatrix != data.m_pModelMatrix.get()) {
			pLastMatrix = data.m_pModelMatrix.get();
			if (pLastMatrix == nullptr) {
				pDraw->ClearModelMatrix();
			}
			else {
				pDraw->SetModelMatrix(*pLastMatrix);
			}
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
