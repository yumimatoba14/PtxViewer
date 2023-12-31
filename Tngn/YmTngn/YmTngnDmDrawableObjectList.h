#pragma once

#include "YmTngnDrawingModel.h"
#include "YmTngnDrawableObject.h"
#include <deque>

namespace Ymcpp {

class YmTngnDmDrawableObjectList : public YmTngnDrawingModel
{
private:
	struct ObjectData {
		YmTngnModelMatrixPtr m_pModelMatrix;
		YmTngnDrawableObjectPtr m_pObject;
	};
public:
	YmTngnDmDrawableObjectList();
	virtual ~YmTngnDmDrawableObjectList();

	void AddObject(YmTngnModelMatrixPtr pMatrix, YmTngnDrawableObjectPtr pObject);
	void ClearObject() { m_objectDataList.clear(); }
	size_t GetCount() const { return m_objectDataList.size(); }

protected:
	//virtual bool OnSetPickEnabled(bool bEnable);
	virtual void OnDraw(YmTngnDraw* pDraw);

private:
	void DrawDirect(YmTngnDraw* pDraw);
	void DrawSorted(YmTngnDraw* pDraw);
	void SortObjectsByDepth(YmTngnDraw* pDraw);
private:
	std::deque<ObjectData> m_objectDataList;
	std::deque<size_t> m_sortedObjectIndexList;
};

}
