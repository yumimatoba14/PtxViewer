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
		DM_PTX_FILE_RADIUS_LOWER_BOUND,
		DM_PTX_FILE_RADIUS_UPPER_BOUND,
		DM_PTX_FILE_MAX_DRAWN_POINT_PER_FRAME_MB,
		DM_PTX_FILE_POINT_COUNT_PER_BLOCK,
		DOUBLE_KEY_COUNT
	};
public:
	YmTngnViewConfig();
	/*virtual ~YmTngnViewConfig();*/

public:
	double GetDoubleValue(DoubleKey key) const { return m_doubleValues[key]; }
	void SetDoubleValue(DoubleKey key, double value) { m_doubleValues[key] = value; }

	int64_t GetDoubleValueAsInt64(DoubleKey key, double coef) const;

	void ReadIniFile(const char* pIniFilePath);
private:
	double m_doubleValues[(int)DOUBLE_KEY_COUNT];
};

}
