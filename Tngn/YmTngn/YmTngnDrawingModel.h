#pragma once

#include "YmTngnFwd.h"

namespace Ymcpp {

class YmTngnDrawingModel
{
protected:
	YmTngnDrawingModel();
public:
	virtual ~YmTngnDrawingModel();

	YmTngnDrawingModel(const YmTngnDrawingModel& other) = delete;
	YmTngnDrawingModel(YmTngnDrawingModel&& other) noexcept = delete;

	YmTngnDrawingModel& operator = (const YmTngnDrawingModel& other) = delete;
	YmTngnDrawingModel& operator = (YmTngnDrawingModel&& other) noexcept = delete;

public:
	void Draw(YmTngnDraw* pDraw);

protected:
	virtual void OnDraw(YmTngnDraw* pDraw) = 0;
};

}
