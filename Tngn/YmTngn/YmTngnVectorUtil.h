#pragma once

#include "YmTngnFwd.h"
#include "YmBase/YmVector.h"

namespace Ymcpp {

namespace Details {
	template<class V>
	struct XmFloat3Traits
	{
		using VectorType = V;
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
			return 0;	// not reached
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

template<>
struct YmVectorTraits<DirectX::XMFLOAT3> : public Details::XmFloat3Traits<DirectX::XMFLOAT3>
{
};

template<>
struct YmVectorTraits<DirectX::XMFLOAT3A> : public Details::XmFloat3Traits<DirectX::XMFLOAT3A>
{
};
}
