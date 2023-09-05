#pragma once

#include "YmBase/YmBaseFwd.h"

namespace Ymcpp {

class YmTngnViewConfig
{
public:
	enum DoubleKey {
		POINT_SIZE = 0,
		FOV_ANGLE_Y_DEG,
		PERSPECTIVE_VIEW_NEAR_Z,
		PERSPECTIVE_VIEW_FAR_Z,
		DOUBLE_KEY_COUNT
	};
public:
	YmTngnViewConfig();
	/*virtual ~YmTngnViewConfig();*/

public:
	double GetDoubleValue(DoubleKey key) const { return m_doubleValues[key]; }
	void SetDoubleValue(DoubleKey key, double value) { m_doubleValues[key] = value; }

private:
	double m_doubleValues[(int)DOUBLE_KEY_COUNT];
};

}
