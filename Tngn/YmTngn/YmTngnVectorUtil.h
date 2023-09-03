#pragma once

#include "YmTngnFwd.h"
#include "YmBase/YmVector.h"

namespace Ymcpp {

template<>
struct YmVectorTraits<DirectX::XMFLOAT3>
{
	using VectorType = DirectX::XMFLOAT3;
	using CoordType = float;
	static constexpr int DIM = 3;
	using DimensionValueType = std::integral_constant<int, DIM>;

	static CoordType GetAt(const VectorType& v, int i)
	{
		switch (i) {
		case 0: return v.x;
		case 1: return v.y;
		case 2: return v.z;
		}
		YM_THROW_ERROR("not supported index");
	}
	static void SetAt(VectorType& v, int i, CoordType coord)
	{
		switch (i) {
		case 0: v.x = coord; break;
		case 1: v.y = coord; break;
		case 2: v.z = coord; break;
		default:
			YM_THROW_ERROR("not supported index");
		}
	}
};
}
