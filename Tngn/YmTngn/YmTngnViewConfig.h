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

	enum IntKey {
		ENABLE_LIGHT = 0,
		INT_KEY_COUNT
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
		MATERIAL_AMBIENT_COEF,
		MATERIAL_DIFFUSE_COEF,
		LIGHT_DIFFUSE_COEF,
		LIGHT_SPECULAR_COEF,
		LIGHT_SPECULAR_SHININESS,
		DOUBLE_KEY_COUNT
	};

	enum Rgba4bKey {
		DEFAULT_TEXT_FG_COLOR = 0,
		DEFAULT_TEXT_BG_COLOR,
		DEFAULT_POLYGON_COLOR,
		LIGHT_DIFFUSE_COLOR,
		LIGHT_SPECULAR_COLOR,
		RGBA4B_KEY_COUNT
	};

	enum Vector3dKey {
		LIGHT_DIR = 0,
		VECTOR3D_KEY_COUNT
	};
public:
	YmTngnViewConfig();
	/*virtual ~YmTngnViewConfig();*/

public:
	YmTString GetStringValue(StringKey key) const { return m_stringValues[key]; }
	void SetStringValue(StringKey key, const YmTString& value) { m_stringValues[key] = value; }

	int GetIntValue(IntKey key) const { return m_intValues[key]; }
	void SetIntValue(IntKey key, int value) { m_intValues[key] = value; }

	double GetDoubleValue(DoubleKey key) const { return m_doubleValues[key]; }
	void SetDoubleValue(DoubleKey key, double value) { m_doubleValues[key] = value; }

	int64_t GetDoubleValueAsInt64(DoubleKey key, double coef) const;

	YmRgba4b GetRgbaValue(Rgba4bKey key) const { return m_rgba4bValues[key]; }
	void SetRgbaValue(Rgba4bKey key, YmRgba4b value) { m_rgba4bValues[key] = value; }

	YmVector3d GetVector3dValue(Vector3dKey key) const { return m_vector3dValues[key]; }
	void SetVector3dValue(Vector3dKey key, const YmVector3d& value) { m_vector3dValues[key] = value; }

public:
	void ReadIniFile(const char* pIniFilePath);
	void DebugOut(std::ostream& out) const;

private:
	YmTString m_stringValues[STRING_KEY_COUNT];
	int m_intValues[INT_KEY_COUNT];
	double m_doubleValues[DOUBLE_KEY_COUNT];
	YmRgba4b m_rgba4bValues[RGBA4B_KEY_COUNT];
	YmVector3d m_vector3dValues[VECTOR3D_KEY_COUNT];
};

}
