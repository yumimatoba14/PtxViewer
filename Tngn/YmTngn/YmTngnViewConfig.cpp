#include "pch.h"
#include "YmTngnViewConfig.h"
#include <vector>

using namespace std;
using namespace Ymcpp;

////////////////////////////////////////////////////////////////////////////////

namespace {
	template<typename ValueType>
	struct ValueSchema {
		const char* keyName;
		ValueType initialValue;
	};

	static const ValueSchema<double> aDoubleValueSchema[YmTngnViewConfig::DOUBLE_KEY_COUNT] = {
		{
			"PointSize", 0.01
		},
		{
			"FovAngleYDeg", 90
		},
		{
			"PerspectiveViewNearZ", 0.01
		},
		{
			"PerspectiveViewFarZ", 100
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

void YmTngnViewConfig::ReadIniFile(const char* pIniFilePath)
{
	YM_IS_TRUE(pIniFilePath != nullptr);
	const char* pAppName = "TngnView";
	vector<char> aBuffer(256);
	size_t nValue = sizeof(aDoubleValueSchema) / sizeof(aDoubleValueSchema[0]);
	for (size_t i = 0; i < nValue; ++i) {
		do {
			DWORD ret = ::GetPrivateProfileStringA(
				pAppName, aDoubleValueSchema[i].keyName, nullptr, aBuffer.data(), (DWORD)aBuffer.size(), pIniFilePath
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
