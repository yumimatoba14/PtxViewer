#pragma once

#include "YmTngnFwd.h"
#include "YmBase/YmAabBox.h"

namespace Ymcpp {

class YmTngnDrawableObject
{
protected:
	YmTngnDrawableObject();
public:
	virtual ~YmTngnDrawableObject();

	YmTngnDrawableObject(const YmTngnDrawableObject& other) = delete;
	YmTngnDrawableObject& operator = (const YmTngnDrawableObject& other) = delete;

	const YmAabBox3d& GetAabBox() const { return m_aabb; }
	void SetAabBox(const YmAabBox3d& box) { m_aabb = box; }

	void Draw(YmTngnDraw* pDraw) { OnDraw(pDraw); }
protected:
	virtual void OnDraw(YmTngnDraw* pDraw) = 0;
private:
	YmAabBox3d m_aabb;
};

}
