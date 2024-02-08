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

/// <summary>
/// Sort object from deep side to shallow side. (Its direction is +Z direction in case of right-hand view coordinate system.)
/// This sorted result is used to draw transparent objects.
/// </summary>
/// <param name="pDraw"></param>
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

		double depth = pDraw->CalculateViewMinDepthForLocalBox(data.m_pObject->GetAabBox(), localToViewMatrix);
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
