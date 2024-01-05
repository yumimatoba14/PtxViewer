#include "pch.h"
#include "YmTngnViewConfig.h"
#include <vector>
#include <ostream>
#include "YmBase/YmDebugOutputStream.h"

using namespace std;
using namespace Ymcpp;

////////////////////////////////////////////////////////////////////////////////

namespace {
	const char* sectionTngnView = "TngnView";
	const char* sectionDmPtxFile = "DmPtxFile";
	template<typename ValueType>
	struct ValueSchema {
		const char* keyName;
		ValueType initialValue;
		const char* sectionName;
	};

	static const ValueSchema<YmTString> aStringValueSchema[YmTngnViewConfig::STRING_KEY_COUNT] = {
		{
			"TextFontName", _T("ÉÅÉCÉäÉI"), sectionTngnView
		},
	};

	static const ValueSchema<int> aIntValueSchema[YmTngnViewConfig::INT_KEY_COUNT] = {
		{
			"EnableLight", 1, sectionTngnView
		},
	};

	static const ValueSchema<double> aDoubleValueSchema[YmTngnViewConfig::DOUBLE_KEY_COUNT] = {
		{
			"PointSize", 0.01, sectionTngnView
		},
		{
			"FovAngleYDeg", 90, sectionTngnView
		},
		{
			"PerspectiveViewNearZ", 0.01, sectionTngnView
		},
		{
			"PerspectiveViewFarZ", 100, sectionTngnView
		},
		{
			"ScannerDistanceUpperBound", 20/*[m]*/, sectionTngnView
		},
		{
			"ScannerDistanceDepthOffset", 0.01/*[no unit, 0 to 1]*/, sectionTngnView
		},
		{
			"RadiusLowerBound", 0.001, sectionDmPtxFile
		},
		{
			"RadiusUpperBound", -1, sectionDmPtxFile
		},
		{
			"MaxDrawnPointPerFrameMega", 1, sectionDmPtxFile
		},
		{
			"BlockPointCountMega", 1, sectionDmPtxFile
		},
		{
			"DefaultTextFontSize", 20, sectionTngnView
		},
		{
			"MaterialAmbientCoef", 0.1, sectionTngnView
		},
		{
			"MaterialDiffuseCoef", 0.6, sectionTngnView
		},
		{
			"LightDiffuseCoef", 0.2, sectionTngnView
		},
		{
			"LightSpecularCoef", 0.2, sectionTngnView
		},
		{
			"LightSpecularShininess", 100, sectionTngnView
		},
	};

	static const ValueSchema<YmRgba4b> aRgba4bValueSchema[YmTngnViewConfig::RGBA4B_KEY_COUNT] = {
		{"DefaultTextFgColor", YmRgba4b(255, 255, 255), sectionTngnView},
		{"DefaultTextBgColor", YmRgba4b(64, 64, 64, 192), sectionTngnView},
		{"LightDiffuseColor", YmRgba4b(255, 255, 255), sectionTngnView},
		{"LightSpecularColor", YmRgba4b(255, 255, 255), sectionTngnView},
	};

	static YmVector3d Vec3d(double x, double y, double z) { return YmVectorUtil::Make(x, y, z); }
	static const ValueSchema<YmVector3d> aVector3dValueSchema[YmTngnViewConfig::VECTOR3D_KEY_COUNT] = {
		{"LightDir", Vec3d(-0.1, -0.1, -1.0), sectionTngnView},
	};
}

YmTngnViewConfig::YmTngnViewConfig()
{
	for (int i = 0; i < STRING_KEY_COUNT; ++i) {
		m_stringValues[i] = aStringValueSchema[i].initialValue;
	}
	for (int i = 0; i < INT_KEY_COUNT; ++i) {
		m_intValues[i] = aIntValueSchema[i].initialValue;
	}
	for (int i = 0; i < DOUBLE_KEY_COUNT; ++i) {
		m_doubleValues[i] = aDoubleValueSchema[i].initialValue;
	}
	for (int i = 0; i < RGBA4B_KEY_COUNT; ++i) {
		m_rgba4bValues[i] = aRgba4bValueSchema[i].initialValue;
	}
	for (int i = 0; i < VECTOR3D_KEY_COUNT; ++i) {
		m_vector3dValues[i] = aVector3dValueSchema[i].initialValue;
	}
}

////////////////////////////////////////////////////////////////////////////////

int64_t YmTngnViewConfig::GetDoubleValueAsInt64(DoubleKey key, double coef) const
{
	double value = GetDoubleValue(key) * coef;
	return int64_t(floor(value + 0.5));
}

static void ReadIniFileString(const char* pIniFilePath, const char* pSection, const char* pKey, vector<char>& aBuffer)
{
	do {
		DWORD ret = ::GetPrivateProfileStringA(
			pSection, pKey, nullptr, aBuffer.data(), (DWORD)aBuffer.size(), pIniFilePath
		);
		if (ret + 1 < aBuffer.size()) {
			break;
		}
		else {
			aBuffer.resize(aBuffer.size() * 2);
		}
	} while (true);
}

static bool ParseRgba(const char* pString, YmRgba4b* pRgba)
{
	int rgba[4];
	char endBracket;
	int numRead = sscanf_s(pString, "(%d,%d,%d%c", rgba + 0, rgba + 1, rgba + 2, &endBracket, 1);
	if (numRead == 4 && endBracket == ')') {
		*pRgba = YmRgba4b(rgba[0], rgba[1], rgba[2]);
		return true;
	}

	numRead = sscanf_s(pString, "(%d,%d,%d,%d%c", rgba + 0, rgba + 1, rgba + 2, rgba + 3, &endBracket, 1);
	if (numRead == 5 && endBracket == ')') {
		*pRgba = YmRgba4b(rgba[0], rgba[1], rgba[2], rgba[3]);
		return true;
	}

	return false;
}

static bool ParseVector(const char* pString, YmVector3d* pValue)
{
	double coord[3];
	char endBracket;
	int numRead = sscanf_s(pString, "(%lg,%lg,%lg%c", coord + 0, coord + 1, coord + 2, &endBracket, 1);
	if (numRead == 4 && endBracket == ')') {
		*pValue = YmVectorUtil::Make(coord[0], coord[1], coord[2]);
		return true;
	}
	return false;
}

void YmTngnViewConfig::ReadIniFile(const char* pIniFilePath)
{
	YM_IS_TRUE(pIniFilePath != nullptr);
	vector<char> aBuffer(256);
	size_t nValue = sizeof(aStringValueSchema) / sizeof(aStringValueSchema[0]);
	for (size_t i = 0; i < nValue; ++i) {
		ReadIniFileString(pIniFilePath, aStringValueSchema[i].sectionName, aStringValueSchema[i].keyName, aBuffer);
		if (strlen(aBuffer.data()) != 0) {
			m_stringValues[i] = ATL::CA2T(aBuffer.data());
		}
	}

	nValue = sizeof(aIntValueSchema) / sizeof(aIntValueSchema[0]);
	for (size_t i = 0; i < nValue; ++i) {
		ReadIniFileString(pIniFilePath, aIntValueSchema[i].sectionName, aIntValueSchema[i].keyName, aBuffer);
		if (strlen(aBuffer.data()) != 0) {
			m_intValues[i] = atoi(aBuffer.data());
		}
	}

	nValue = sizeof(aDoubleValueSchema) / sizeof(aDoubleValueSchema[0]);
	for (size_t i = 0; i < nValue; ++i) {
		do {
			const char* pAppName = aDoubleValueSchema[i].sectionName;
			const char* pKey = aDoubleValueSchema[i].keyName;
			DWORD ret = ::GetPrivateProfileStringA(
				pAppName, pKey, nullptr, aBuffer.data(), (DWORD)aBuffer.size(), pIniFilePath
			);
			if (ret + 1 < aBuffer.size()) {
				double value = 0;
				int nRead = sscanf_s(aBuffer.data(), "%lg", &value);
				if (nRead == 1) {
					m_doubleValues[i] = value;
				}
				break;
			}
			else {
				aBuffer.resize(aBuffer.size() * 2);
			}
		} while (true);
	}

	nValue = sizeof(aRgba4bValueSchema) / sizeof(aRgba4bValueSchema[0]);
	for (size_t i = 0; i < nValue; ++i) {
		ReadIniFileString(pIniFilePath, aRgba4bValueSchema[i].sectionName, aRgba4bValueSchema[i].keyName, aBuffer);
		YmRgba4b value;
		bool isRead = ParseRgba(aBuffer.data(), &value);
		if (isRead) {
			m_rgba4bValues[i] = value;
		}
	}

	nValue = sizeof(aVector3dValueSchema) / sizeof(aVector3dValueSchema[0]);
	for (size_t i = 0; i < nValue; ++i) {
		ReadIniFileString(pIniFilePath, aVector3dValueSchema[i].sectionName, aVector3dValueSchema[i].keyName, aBuffer);
		YmVector3d value;
		bool isRead = ParseVector(aBuffer.data(), &value);
		if (isRead) {
			m_vector3dValues[i] = value;
		}
	}

#if defined(_DEBUG)
	DebugOut(YmDebugOutputStream());
#endif
}

namespace {
	ostream& operator << (ostream& out, const YmTString& value)
	{
		using namespace ATL;
		out << static_cast<const char*>(CT2A(value.c_str()));
		return out;
	}

	ostream& operator << (ostream& out, const YmRgba4b& value)
	{
		out << "(" << value.GetR() << ", " << value.GetG() << ", " << value.GetB() << ", " << value.GetA() << ")";
		return out;
	}

	template<class C>
	ostream& operator << (ostream& out, const YmVectorN<3,C>& value)
	{
		out << "(" << value[0] << ", " << value[1] << ", " << value[2] << ")";
		return out;
	}

	template<class SCHEMA, class VALUE>
	void DebugOutValues(ostream& out, int nValue, const SCHEMA aSchema[], const VALUE aValue[])
	{
		for (int i = 0; i < nValue; ++i) {
			out << "[" << aSchema[i].sectionName << "]" << aSchema[i].keyName << "=" << aValue[i] << endl;
		}
	}
}

void YmTngnViewConfig::DebugOut(std::ostream& out) const
{
	DebugOutValues(out, sizeof(aStringValueSchema) / sizeof(aStringValueSchema[0]), aStringValueSchema, m_stringValues);
	DebugOutValues(out, sizeof(aIntValueSchema) / sizeof(aIntValueSchema[0]), aIntValueSchema, m_intValues);
	DebugOutValues(out, sizeof(aDoubleValueSchema) / sizeof(aDoubleValueSchema[0]), aDoubleValueSchema, m_doubleValues);
	DebugOutValues(out, sizeof(aRgba4bValueSchema) / sizeof(aRgba4bValueSchema[0]), aRgba4bValueSchema, m_rgba4bValues);
	DebugOutValues(out, sizeof(aVector3dValueSchema) / sizeof(aVector3dValueSchema[0]), aVector3dValueSchema, m_vector3dValues);
}

////////////////////////////////////////////////////////////////////////////////
