#include "pch.h"
#include "YmTngnViewConfig.h"

using namespace std;
using namespace Ymcpp;

////////////////////////////////////////////////////////////////////////////////

namespace {
	template<typename ValueType>
	struct ValueSchema {
		const char* keyName;
		ValueType initialValue;
	};

	static const ValueSchema<double> doubleValueSchema[YmTngnViewConfig::DOUBLE_KEY_COUNT] = {
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
		m_doubleValues[i] = doubleValueSchema[i].initialValue;
	}
}

////////////////////////////////////////////////////////////////////////////////
