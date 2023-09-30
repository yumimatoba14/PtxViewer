#pragma once

#include "YmBaseFwd.h"
#include "YmVectorN.h"
#include <type_traits>
//#include <cmath>

namespace Ymcpp {

////////////////////////////////////////////////////////////////////////////////

typedef YmVectorN<2, double> YmVector2d;
typedef YmVectorN<3, double> YmVector3d;
typedef YmVectorN<2, float> YmVector2f;
typedef YmVectorN<3, float> YmVector3f;
typedef YmVectorN<2, int> YmVector2i;
typedef YmVectorN<3, int> YmVector3i;

////////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////////

// Utility class for YmVectorN.
namespace YmVectorUtil
{
	template<class COORD>
	static YmVectorN<2, COORD> Make(COORD x, COORD y)
	{
		const COORD aCoord[2] = { x, y };
		return YmVectorN<2, COORD>(aCoord);
	}

	template<class COORD>
	static YmVectorN<3, COORD> Make(COORD x, COORD y, COORD z)
	{
		const COORD aCoord[3] = { x, y, z };
		return YmVectorN<3, COORD>(aCoord);
	}

	template<class V>
	static void CopyToArray(const V& vec, int nCoord, typename YmVectorTraits<V>::CoordType aCoord[])
	{
		YM_IS_TRUE(nCoord <= YmVectorTraits<V>::DIM);
		for (int i = 0; i < nCoord; ++i) {
			aCoord[i] = YmVectorTraits<V>::GetAt(vec, i);
		}
	}

	namespace Detail {
		template<class VRet>
		const VRet& StaticCastImpl(const VRet& vecFrom, std::true_type isSameType) { return vecFrom; }

		template<class VRet, class VIn>
		static VRet StaticCastImpl(const VIn& vecFrom, std::false_type isSameType)
		{
			typename YmVectorTraits<VRet>::DimensionValueType dim = typename YmVectorTraits<VIn>::DimensionValueType();
			VRet ret;
			for (int i = 0; i < dim; ++i) {
				YmVectorTraits<VRet>::SetAt(ret, i,
					static_cast<typename YmVectorTraits<VRet>::CoordType>(YmVectorTraits<VIn>::GetAt(vecFrom, i))
				);
			}
			return ret;
		}
	}

	template<class VRet, class VIn>
	static VRet StaticCast(const VIn& vecFrom)
	{
		return Detail::StaticCastImpl<VRet>(vecFrom, typename std::is_same<std::decay_t<VRet>, std::decay_t<VIn>>::type());
	}

	template<class V>
	typename YmVectorTraits<V>::CoordType InnerProduct(const V& lhs, const V& rhs)
	{
		using VectorType = YmVectorN<YmVectorTraits<V>::DIM, typename YmVectorTraits<V>::CoordType>;
		return static_cast<const VectorType&>(lhs).Dot(rhs);
	}

	namespace Detail {
		/// <summary>
		/// 
		/// </summary>
		/// <typeparam name="V"></typeparam>
		/// <param name="lhs"></param>
		/// <param name="rhs"></param>
		/// <param name="dim">for validation of dimension. It must be 3.</param>
		/// <returns></returns>
		template<class V>
		V OuterProductImpl(const V& lhs, const V& rhs, std::integral_constant<int, 3> dim = typename YmVectorTraits<V>::DimensionValueType())
		{
			using VTraits = YmVectorTraits<V>;
			V retValue;
			VTraits::SetAt(retValue, 0,
				VTraits::GetAt(lhs, 1) * VTraits::GetAt(rhs, 2) - VTraits::GetAt(lhs, 2) * VTraits::GetAt(rhs, 1)
			);
			VTraits::SetAt(retValue, 1,
				VTraits::GetAt(lhs, 2) * VTraits::GetAt(rhs, 0) - VTraits::GetAt(lhs, 0) * VTraits::GetAt(rhs, 2)
			);
			VTraits::SetAt(retValue, 2,
				VTraits::GetAt(lhs, 0) * VTraits::GetAt(rhs, 1) - VTraits::GetAt(lhs, 1) * VTraits::GetAt(rhs, 0)
			);
			return retValue;
		}

		template<int N>
		bool NormalizeVectorImpl(double zeroTol, const YmVectorN<N, double>& vec, YmVectorN<N, double>* pResult)
		{
			YM_IS_TRUE(0 < zeroTol);
			double sqLen = vec.GetSqLength();
			if (sqLen <= zeroTol * zeroTol) {
				if (pResult != nullptr) {
					*pResult = vec;
				}
				return false;
			}
			if (pResult != nullptr) {
				*pResult = vec * (1.0 / sqrt(sqLen));
			}
			return true;
		}
	}

	template<class V>
	V OuterProduct(const V& lhs, const V& rhs)
	{
		return Detail::OuterProductImpl(lhs, rhs);
	}

	template<int N>
	bool TryNormalize(double zeroTol, const YmVectorN<N, float>& vec, YmVectorN<N, float>* pResult)
	{
		if (pResult == nullptr) {
			return Detail::NormalizeVectorImpl(zeroTol, vec, nullptr);
		}
		YmVectorN<N, double> doubleResult;
		bool ok = Detail::NormalizeVectorImpl(zeroTol, vec, &doubleResult);
		*pResult = YmVectorUtil::StaticCast<YmVectorN<N, double>>(doubleResult);
		return ok;
	}

	template<int N>
	bool TryNormalize(double zeroTol, const YmVectorN<N, float>& vec, nullptr_t)
	{
		return Detail::NormalizeVectorImpl<N>(zeroTol, vec, nullptr);
	}

	/// <summary>
	/// try to normalize a 3D vector.
	/// </summary>
	/// <param name="zeroTol"></param>
	/// <param name="vec"></param>
	/// <param name="pResult">specify not null to get normalized result. pResult can be &vec.
	/// pResult can be nullptr if the result is not necessary.</param>
	/// <returns>false if vec's length is smaller than zeroTol.</returns>
	template<int N>
	bool TryNormalize(double zeroTol, const YmVectorN<N, double>& vec, YmVectorN<N, double>* pResult)
	{
		return Detail::NormalizeVectorImpl(zeroTol, vec, pResult);
	}

	template<int N>
	bool TryNormalize(double zeroTol, const YmVectorN<N, double>& vec, nullptr_t)
	{
		return Detail::NormalizeVectorImpl<N>(zeroTol, vec, nullptr);
	}

	template<int N>
	YmVectorN<N,double> Normalize(double zeroTol, const YmVectorN<N, double>& vec)
	{
		YmVectorN<N, double> result;
		if (!Detail::NormalizeVectorImpl(zeroTol, vec, &result)) {
			YM_THROW_ERROR("A zero length vector cannot be normalized.");
		}
		return result;
	}

	template<int N>
	YmVectorN<N, float> Normalize(double zeroTol, const YmVectorN<N, float>& vec)
	{
		return StaticCast<YmVectorN<N, float>>(Normalize(zeroTol, (YmVectorN<N, double>)vec));
	}
};

}
