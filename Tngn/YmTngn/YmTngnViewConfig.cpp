#include "pch.h"
#include "YmTngnViewConfig.h"
#include <vector>

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
	};

	static const ValueSchema<YmRgba4b> aRgba4bValueSchema[YmTngnViewConfig::RGBA4B_KEY_COUNT] = {
		{"DefaultTextFgColor", YmRgba4b(255, 255, 255), sectionTngnView},
		{"DefaultTextBgColor", YmRgba4b(64, 64, 64, 192), sectionTngnView},
	};
}

YmTngnViewConfig::YmTngnViewConfig()
{
	for (int i = 0; i < STRING_KEY_COUNT; ++i) {
		m_stringValues[i] = aStringValueSchema[i].initialValue;
	}
	for (int i = 0; i < DOUBLE_KEY_COUNT; ++i) {
		m_doubleValues[i] = aDoubleValueSchema[i].initialValue;
	}
	for (int i = 0; i < RGBA4B_KEY_COUNT; ++i) {
		m_rgba4bValues[i] = aRgba4bValueSchema[i].initialValue;
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
}

////////////////////////////////////////////////////////////////////////////////
