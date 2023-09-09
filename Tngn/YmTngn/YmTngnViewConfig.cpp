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
	};
}

YmTngnViewConfig::YmTngnViewConfig()
{
	for (int i = 0; i < DOUBLE_KEY_COUNT; ++i) {
		m_doubleValues[i] = aDoubleValueSchema[i].initialValue;
	}
}

////////////////////////////////////////////////////////////////////////////////

int64_t YmTngnViewConfig::GetDoubleValueAsInt64(DoubleKey key, double coef) const
{
	double value = GetDoubleValue(key) * coef;
	return int64_t(floor(value + 0.5));
}

void YmTngnViewConfig::ReadIniFile(const char* pIniFilePath)
{
	YM_IS_TRUE(pIniFilePath != nullptr);
	const char* pAppName = "TngnView";
	vector<char> aBuffer(256);
	size_t nValue = sizeof(aDoubleValueSchema) / sizeof(aDoubleValueSchema[0]);
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
}

////////////////////////////////////////////////////////////////////////////////
