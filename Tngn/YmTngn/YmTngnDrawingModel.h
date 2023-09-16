#pragma once

#include "YmTngnFwd.h"
#include <vector>

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
	bool IsPickEnabled() const { return m_isPickEnabled; }

	/// <summary>
	/// 
	/// </summary>
	/// <returns>Whether pick is enabled.</returns>
	bool SetPickEnabled(bool isEnable);
	void Draw(YmTngnDraw* pDraw);
	std::vector<YmTngnPointListVertex> FindPickedPoints(YmTngnPickTargetId id);

protected:
	virtual bool OnSetPickEnabled(bool bEnable);
	virtual void OnDraw(YmTngnDraw* pDraw) = 0;
	virtual std::vector<YmTngnPointListVertex> OnFindPickedPoints(YmTngnPickTargetId id);

private:
	bool m_isPickEnabled = false;
};

}
