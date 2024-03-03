#pragma once

#include "YmBaseFwd.h"

namespace Ymcpp {

template<class V>
struct YmVectorTraits;

template<int N, typename COORD = double>
class YmVectorN;

////////////////////////////////////////////////////////////////////////////////

// This general YmVectorTraits class definition is disabled to remove
//  YmVectorN(const V& v) overloads for invalid V.
#if 0
/// <summary>
/// This class implicitly implements traits for YmVectorN.
/// </summary>
/// <typeparam name="V"></typeparam>
template<typename V>
struct YmVectorTraits
{
	using VectorType = V;
	using CoordType = typename V::CoordType;
	static constexpr int DIM = V::DIM;
	using DimensionValueType = std::integral_constant<int, DIM>;

	static CoordType GetAt(const VectorType& v, int i) { return v[i]; }
	static void SetAt(VectorType& v, int i, CoordType coord) { v[i] = coord; }
};
#endif

////////////////////////////////////////////////////////////////////////////////

using YmVector2d = YmVectorN<2, double>;
using YmVector3d = YmVectorN<3, double>;
using YmVector2f = YmVectorN<2, float>;
using YmVector3f = YmVectorN<3, float>;
using YmVector2i = YmVectorN<2, int>;
using YmVector3i = YmVectorN<3, int>;

////////////////////////////////////////////////////////////////////////////////

}
