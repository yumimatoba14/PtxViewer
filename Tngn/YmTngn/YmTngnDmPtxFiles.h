#pragma once

#include "YmTngnDrawingModel.h"
#include "YmTngnDmPointBlockList.h"
#include "YmTngnViewConfig.h"
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
	explicit YmTngnDmPtxFiles(const YmTngnViewConfig& config);
	virtual ~YmTngnDmPtxFiles();

	YmTString ReadPtxFile(const YmTString& filePath);
protected:
	virtual bool OnSetPickEnabled(bool bEnable);
	virtual void OnDraw(YmTngnDraw* pDraw);
	virtual std::vector<YmTngnPointListVertex> OnFindPickedPoints(YmTngnPickTargetId id);

private:
	YmTngnViewConfig m_config;
	YmTngnDmPointBlockList m_blockListImpl;
	std::vector<Content> m_contents;
};

}
