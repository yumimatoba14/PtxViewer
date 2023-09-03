#pragma once

#include "YmBaseFwd.h"

namespace Ymcpp {

template<class V>
struct YmVectorTraits;

template<int N, typename COORD = double>
class YmVectorN
{
public:
	using CoordType = COORD;
	static constexpr int DIM = N;

	// This default constructor doesn't initialize this members
	// so as to omit initialization. It is not so harmfull for this class.
	YmVectorN() {}

	YmVectorN(const YmVectorN&) = default;

	explicit YmVectorN(const CoordType aCoord[DIM])
	{
		for (int i = 0; i < DIM; ++i) {
			m_aCoord[i] = aCoord[i];
		}
	}

	template<typename C>
	explicit YmVectorN(const C aCoord[DIM])
	{
		for (int i = 0; i < DIM; ++i) {
			m_aCoord[i] = aCoord[i];
		}
	}

	template<typename V, typename TRAITS = YmVectorTraits<V>>
	YmVectorN(const V& v)
	{
		using DimensionValueType = typename YmVectorTraits<YmVectorN>::DimensionValueType;
		DimensionValueType dim = typename TRAITS::DimensionValueType();
		for (int i = 0; i < DIM; ++i) {
			m_aCoord[i] = TRAITS::GetAt(v, i);
		}
	}

	YmVectorN& operator = (const YmVectorN&) = default;

	const CoordType* ToArray() const { return m_aCoord; }

	CoordType& operator [](int i) { return m_aCoord[i]; }
	CoordType operator [](int i) const { return m_aCoord[i]; }

	YmVectorN& operator += (const YmVectorN& rhs)
	{
		for (int i = 0; i < DIM; ++i) {
			m_aCoord[i] += rhs[i];
		}
		return *this;
	}

	YmVectorN& operator -= (const YmVectorN& rhs)
	{
		for (int i = 0; i < DIM; ++i) {
			m_aCoord[i] -= rhs[i];
		}
		return *this;
	}

	YmVectorN& operator *= (CoordType scalar)
	{
		for (int i = 0; i < DIM; ++i) {
			m_aCoord[i] *= scalar;
		}
		return *this;
	}


	CoordType Dot(const YmVectorN& rhs) const
	{
		CoordType ret = 0;
		for (int i = 0; i < DIM; ++i) {
			ret += m_aCoord[i] * rhs.m_aCoord[i];
		}
		return ret;
	}

	CoordType GetSqLength() const { return Dot(*this); }

	CoordType GetLength() const { return sqrt(GetSqLength()); }

	static YmVectorN MakeZero()
	{
		YmVectorN vec;
		for (int i = 0; i < DIM; ++i) {
			vec[i] = 0;
		}
		return vec;
	}

	/// <summary>
	/// 
	/// </summary>
	/// <param name="rhs"></param>
	/// <returns>0 if this is equal to rhs.
	/// returns < 0 if this is smaller than rhs.
	/// returns > 0 if this is bigger than rhs.</returns>
	int Compare(const YmVectorN& rhs) const
	{
		for (int i = 0; i < DIM; ++i) {
			if (m_aCoord[i] != rhs.m_aCoord[i]) {
				return (m_aCoord[i] < rhs.m_aCoord[i] ? -1 : 1);
			}
		}
		return 0;
	}

private:
	CoordType m_aCoord[DIM];
};

////////////////////////////////////////////////////////////////////////////////


template<int N, typename COORD>
YmVectorN<N, COORD> operator + (const YmVectorN<N, COORD>& lhs, const YmVectorN<N, COORD>& rhs)
{
	return YmVectorN<N, COORD>(lhs) += rhs;
}

template<int N, typename COORD>
YmVectorN<N, COORD> operator - (const YmVectorN<N, COORD>& lhs, const YmVectorN<N, COORD>& rhs)
{
	return YmVectorN<N, COORD>(lhs) -= rhs;
}

template<int N, typename COORD, typename SCALAR>
YmVectorN<N, COORD> operator * (SCALAR s, const YmVectorN<N, COORD>& v)
{
	return YmVectorN<N, COORD>(v) *= (COORD)s;
}

template<int N, typename COORD, typename SCALAR>
YmVectorN<N, COORD> operator * (const YmVectorN<N, COORD>& v, SCALAR s)
{
	return YmVectorN<N, COORD>(v) *= (COORD)s;
}

template<int N, typename COORD>
bool operator < (const YmVectorN<N, COORD>& lhs, const YmVectorN<N, COORD>& rhs)
{
	return lhs.Compare(rhs) < 0;
}

template<int N, typename COORD>
bool operator <= (const YmVectorN<N, COORD>& lhs, const YmVectorN<N, COORD>& rhs)
{
	return lhs.Compare(rhs) <= 0;
}

template<int N, typename COORD>
bool operator == (const YmVectorN<N, COORD>& lhs, const YmVectorN<N, COORD>& rhs)
{
	return lhs.Compare(rhs) == 0;
}

template<int N, typename COORD>
bool operator != (const YmVectorN<N, COORD>& lhs, const YmVectorN<N, COORD>& rhs)
{
	return lhs.Compare(rhs) != 0;
}

////////////////////////////////////////////////////////////////////////////////

}
