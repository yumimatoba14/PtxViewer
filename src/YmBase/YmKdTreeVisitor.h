#pragma once

#include "YmBaseFwd.h"
#include "YmVector.h"

namespace Ymcpp {

class YmKdTreeVisitor
{
public:
	enum class Order {
		STOP,
		SMALLER_FIRST,
		BIGGER_FIRST,
		DEFAULT
	};

	/// <summary>
	/// Retuns whether this area shall be visisted on traversing a tree.
	/// The area is represented as an AABB.
	/// </summary>
	/// <typeparam name="POINT">YmVectors or their compatible class to support YmVectorTraits.</typeparam>
	/// <param name="lowerBound">lower bound point of AABB.</param>
	/// <param name="upperBound">upper bound pont of AABB</param>
	/// <returns>true to visit this box.</returns>
	template<class POINT>
	bool IsVisitThisBox(const POINT& lowerBound, const POINT& upperBound) const { return true; }

	/// <summary>
	/// 
	/// </summary>
	/// <typeparam name="CONTENT"></typeparam>
	/// <param name="c"></param>
	/// <returns>false to stop traversing the tree.</returns>
	template<class CONTENT, class POINT>
	bool OnVisitContent(const CONTENT& c, const POINT& contentPoint) { return true; }

	/// <summary>
	/// Decide node order to be visited on traversing.
	/// </summary>
	/// <typeparam name="POINT"></typeparam>
	/// <param name="axisIndex"></param>
	/// <param name="threshold"></param>
	/// <param name="lowerBound"></param>
	/// <param name="upperBound"></param>
	/// <returns></returns>
	template<class POINT>
	Order DecideVisitingOrder(
		int splittingAxisIndex, typename YmVectorTraits<POINT>::CoordType threshold,
		const POINT& lowerBound, const POINT& upperBound
	) const
	{
		return Order::DEFAULT;
	}
};

////////////////////////////////////////////////////////////////////////////////

template<class CONTENT, class POINT>
class YmKdTreeNearestPointFinder : public YmKdTreeVisitor
{
public:
	using PointType = POINT;
	using ContentType = CONTENT;
	using CoordType = typename YmVectorTraits<PointType>::CoordType;

	explicit YmKdTreeNearestPointFinder(const PointType& point) : m_givenPoint(point), m_sqDistanceMin(-1)
	{}

	bool IsFoundNearestContent() const { return 0 <= m_sqDistanceMin; }
	const ContentType& GetNearestContent() const { return m_nearestContent; }

	bool IsVisitThisBox(const PointType& lowerBound, const PointType& upperBound) const
	{
		if (!IsFoundNearestContent()) {
			return true;
		}
		PointType diff = YmAabBox<PointType>::CalculateDifferenceFromOuterPoint(lowerBound, upperBound, m_givenPoint);
		CoordType sqDist = diff.GetSqLength();
		return sqDist < m_sqDistanceMin;
	}

	bool OnVisitContent(const ContentType& c, const PointType& contentPoint)
	{
		CoordType sqDist = (contentPoint - m_givenPoint).GetSqLength();
		if (!IsFoundNearestContent() || sqDist < m_sqDistanceMin) {
			m_sqDistanceMin = sqDist;
			m_nearestContent = c;
		}
		return true;
	}

	Order DecideVisitingOrder(
		int splittingAxisIndex, CoordType threshold, const PointType& lowerBound, const PointType& upperBound
	) const
	{
		if (m_givenPoint[splittingAxisIndex] < threshold) {
			return Order::SMALLER_FIRST;
		}
		else if (threshold < m_givenPoint[splittingAxisIndex]) {
			return Order::BIGGER_FIRST;
		}
		return Order::DEFAULT;
	}

private:
	PointType m_givenPoint;
	ContentType m_nearestContent;
	CoordType m_sqDistanceMin = -1;
};

}
