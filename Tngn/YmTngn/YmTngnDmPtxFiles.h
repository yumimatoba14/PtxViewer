#pragma once

#include "YmTngnDrawingModel.h"
#include "YmTngnDmPointBlockList.h"
#include <vector>

namespace Ymcpp {

class YmTngnDmPtxFiles : public YmTngnDrawingModel
{
private:
	struct Content {
		YmTString name;
		bool isActive;
		YmTngnDrawingModelPtr pModel;
	};
public:
	YmTngnDmPtxFiles();
	virtual ~YmTngnDmPtxFiles();

	YmTString ReadPtxFile(const YmTString& filePath);
protected:
	virtual void OnDraw(YmTngnDraw* pDraw);

private:
	YmTngnDmPointBlockList m_blockListImpl;
	std::vector<Content> m_contents;
};

}
