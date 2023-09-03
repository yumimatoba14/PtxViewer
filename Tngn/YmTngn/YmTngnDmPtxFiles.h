#pragma once

#include "YmTngnDrawingModel.h"
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
	void AddChildModel(const YmTString& name, const YmTngnDrawingModelPtr& pModel);
protected:
	virtual void OnDraw(YmTngnDraw* pDraw);

private:
	std::vector<Content> m_contents;
};

}
