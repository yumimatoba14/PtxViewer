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

	bool IsDrawWithScannerPosition() const { return m_isDrawWithScannerPosition; }
	void SetDrawWithScannerPosition(bool isUse);

	YmTString ReadPtxFile(const YmTString& filePath);

	std::vector<YmTngnPickedPoint> FindPickedPoints(YmTngnPickTargetId id) const;
protected:
	virtual bool OnSetPickEnabled(bool bEnable);
	virtual void OnDraw(YmTngnDraw* pDraw);

private:
	YmTngnViewConfig m_config;
	YmTngnDmPointBlockList m_blockListImpl;
	std::vector<Content> m_contents;
	bool m_isDrawWithScannerPosition = true;
};

}
