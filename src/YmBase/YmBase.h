#pragma once

#include "YmBaseFwd.h"
#include <utility>

namespace Ymcpp {
	template<class T1, class T2>
	void MoveToLeft(T1& left, T1& right, T2&& iniValue)
	{
		left = std::move(right);
		right = std::forward<T2>(iniValue);
	}

}
