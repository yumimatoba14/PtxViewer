#pragma once

#include "YmBase/YmBaseFwd.h"
#include "YmTngnModel/YmRgba.h"

namespace Ymcpp {

class YmTngnViewConfig
{
public:
	enum StringKey {
		TEXT_FONT_NAME = 0,
		STRING_KEY_COUNT
	};

	enum DoubleKey {
		POINT_SIZE = 0,
		FOV_ANGLE_Y_DEG,
		PERSPECTIVE_VIEW_NEAR_Z,
		PERSPECTIVE_VIEW_FAR_Z,
		SCANNER_DISTANCE_UB,
		SCANNER_DISTANCE_DEPTH_OFFSET,
		DM_PTX_FILE_RADIUS_LOWER_BOUND,
		DM_PTX_FILE_RADIUS_UPPER_BOUND,
		DM_PTX_FILE_MAX_DRAWN_POINT_PER_FRAME_MB,
		DM_PTX_FILE_POINT_COUNT_PER_BLOCK,
		DEFAULT_TEXT_FONT_SIZE,
		DOUBLE_KEY_COUNT
	};

	enum Rgba4bKey {
		DEFAULT_TEXT_FG_COLOR = 0,
		DEFAULT_TEXT_BG_COLOR,
		RGBA4B_KEY_COUNT
	};
public:
	YmTngnViewConfig();
	/*virtual ~YmTngnViewConfig();*/

public:
	YmTString GetStringValue(StringKey key) const { return m_stringValues[key]; }
	void SetStringValue(StringKey key, const YmTString& value) { m_stringValues[key] = value; }

	double GetDoubleValue(DoubleKey key) const { return m_doubleValues[key]; }
	void SetDoubleValue(DoubleKey key, double value) { m_doubleValues[key] = value; }

	int64_t GetDoubleValueAsInt64(DoubleKey key, double coef) const;

	YmRgba4b GetRgbaValue(Rgba4bKey key) const { return m_rgba4bValues[key]; }
	void SetRgbaValue(Rgba4bKey key, YmRgba4b value) { m_rgba4bValues[key] = value; }

public:
	void ReadIniFile(const char* pIniFilePath);

private:
	YmTString m_stringValues[(int)STRING_KEY_COUNT];
	double m_doubleValues[(int)DOUBLE_KEY_COUNT];
	YmRgba4b m_rgba4bValues[(int)RGBA4B_KEY_COUNT];
};

}
