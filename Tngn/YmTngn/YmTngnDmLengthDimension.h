#pragma once

#include "YmTngn.h"
#include "YmTngnDrawingModel.h"
#include "YmTngnModel/YmRgba.h"

namespace Ymcpp {

class YmTngnDmLengthDimension : public YmTngnDrawingModel
{
public:
	YmTngnDmLengthDimension();
	YmTngnDmLengthDimension(const YmVector3d& point0, const YmVector3d& point1);
	virtual ~YmTngnDmLengthDimension();

public:
	YmRgba4b GetColor() const { return m_color; }
	void SetColor(const YmRgba4b& color);

	YmVector3d GetPointAt(int i) const { YM_IS_TRUE(0 <= i && i < 2); return YmVector3d(m_aTerminalPoint[i].position); }
	void SetPointAt(int i, const YmVector3d& coord);

	void SetLengthText(double length);
protected:
	//virtual bool OnSetPickEnabled(bool bEnable);
	virtual void OnDraw(YmTngnDraw* pDraw);
	//virtual std::vector<YmTngnPointListVertex> OnFindPickedPoints(YmTngnPickTargetId id);

private:
	YmRgba4b m_color;
	YmTngnPointListVertex m_aTerminalPoint[2];
	std::string m_text;
};

}
