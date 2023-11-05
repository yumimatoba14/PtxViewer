#include "pch.h"
#include "YmTngnDmDrawableObjectList.h"
#include "YmTngnDraw.h"

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

////////////////////////////////////////////////////////////////////////////////
