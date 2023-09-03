#pragma once

#include "YmVector.h"

namespace Ymcpp {

/// <summary>
/// 
/// </summary>
/// <typeparam name="V">YmVectorN or its compatible class.</typeparam>
template<typename V>
class YmAabBox
{
public:
	using VectorType = V;
	using CoordType = typename YmVectorTraits<VectorType>::CoordType;
	static constexpr int DIM = YmVectorTraits<VectorType>::DIM;

	YmAabBox()
	{
		m_minPoint[0] = 0;
		m_maxPoint[0] = -1;
	}

	bool IsInitialized() const { return m_minPoint[0] <= m_maxPoint[0]; }

	const VectorType& GetMinPoint() const { return m_minPoint; }
	const VectorType& GetMaxPoint() const { return m_maxPoint; }

	bool IsInclude(const VectorType& point)
	{
		return IsInclude(point.ToArray());
	}

	bool IsInclude(const CoordType aCoord[DIM])
	{
		if (!IsInitialized()) {
			return false;
		}
		for (int i = 0; i < DIM; ++i) {
			if (aCoord[i] < m_minPoint[i]) {
				return false;
			}
			else if (m_maxPoint[i] < aCoord[i]) {
				return false;
			}
		}
		return true;
	}

	void Extend(const VectorType& point)
	{
		Extend(point.ToArray());
	}

	void Extend(const CoordType aCoord[DIM])
	{
		if (!IsInitialized()) {
			m_minPoint = m_maxPoint = VectorType(aCoord);
		}
		else {
			for (int i = 0; i < DIM; ++i) {
				if (aCoord[i] < m_minPoint[i]) {
					m_minPoint[i] = aCoord[i];
				}
				else if (m_maxPoint[i] < aCoord[i]) {
					m_maxPoint[i] = aCoord[i];
				}
			}
		}
	}

	static VectorType CalculateDifferenceFromOuterPoint(const VectorType& minPoint, const VectorType& maxPoint, const VectorType& outerPoint)
	{
		VectorType diff;
		for (int i = 0; i < DIM; ++i) {
			diff[i] = CalcCoordDifference(minPoint[i], maxPoint[i], outerPoint[i]);
		}
		return diff;
	}

private:
	static CoordType CalcCoordDifference(CoordType minPoint, CoordType maxPoint, CoordType outerPoint)
	{
		if (outerPoint < minPoint) {
			return minPoint - outerPoint;
		}
		else if (maxPoint < outerPoint) {
			return maxPoint - outerPoint;
		}
		return 0;
	}

private:
	VectorType m_minPoint;
	VectorType m_maxPoint;
};

typedef YmAabBox<YmVector2d> YmAabBox2d;
typedef YmAabBox<YmVector3d> YmAabBox3d;

}
