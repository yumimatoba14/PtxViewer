#pragma once

#include "YmBaseFwd.h"
#include "YmAabBox.h"
#include "YmKdTreeVisitor.h"
#include <deque>
#include <vector>
#include <algorithm>

namespace Ymcpp {

template<class CONTENT, class POINT>
class YmKdTreeContentPolicy
{
public:
	using ContentType = CONTENT;
	using PointType = POINT;

	static PointType GetPoint(const ContentType& c) { return (PointType)c; }
};

class YmKdTreeConstructionPolicy
{
public:
	template<class POINT>
	static int GetNextSplittingAxis(int currAxisIndex, const POINT& lowerBound, const POINT& upperBound)
	{
		return GetLongestAxis(upperBound - lowerBound);
	}

	// utility functions to implement ConstructionPolicy.
public:
	static int GetNextAxisIndex(int currAxisIndex, int dim)
	{
		return (currAxisIndex + 1) % dim;
	}

	template<class POINT>
	static int GetLongestAxis(const POINT& diff)
	{
		int found = 0;
		YmVectorTraits<POINT>::CoordType maxLength = diff[0];
		for (int i = 1; i < YmVectorTraits<POINT>::DIM; ++i) {
			if (maxLength < diff[i]) {
				found = i;
				maxLength = diff[i];
			}
		}
		return found;
	}
};

template<class CONTENT_POLICY, class CONSTRUCTION_POLICY = YmKdTreeConstructionPolicy>
class YmKdTree : private CONTENT_POLICY, private CONSTRUCTION_POLICY
{
public:
	using ContentPolicy = CONTENT_POLICY;
	using ContentType = typename ContentPolicy::ContentType;
	using PointType = YmVectorN<YmVectorTraits<typename ContentPolicy::PointType>::DIM, typename YmVectorTraits<typename ContentPolicy::PointType>::CoordType>;
	using CoordType = typename YmVectorTraits<PointType>::CoordType;
	static constexpr int DIM = YmVectorTraits<PointType>::DIM;
	using ConstructionPolicy = CONSTRUCTION_POLICY;

private:
	using NodeHandle = size_t;
	static constexpr NodeHandle INVALID_NODE_HANDLE = -1;
	struct Node
	{
		int axisIndex = -1;
		CoordType threshold = 0;
		NodeHandle smallerNodeIndex = INVALID_NODE_HANDLE;
		NodeHandle biggerNodeIndex = INVALID_NODE_HANDLE;
		ContentType content;

		Node() {}
		Node(const ContentType& c) : content(c) {}
	};

	class NodeComparator : private ContentPolicy
	{
	public:
		explicit NodeComparator(int axis) : m_axis(axis) {}
		bool operator () (const Node& left, const Node& right) const
		{
			return ContentPolicy::GetPoint(left.content)[m_axis] < ContentPolicy::GetPoint(right.content)[m_axis];
		}
	private:
		int m_axis;
	};

public:
	void AddContent(const ContentType& c)
	{
		PointType point = ContentPolicy::GetPoint(c);
		m_nodes.push_back(c);
		m_treeAabb.Extend(point);
		InvalidateTree();
	}

	const YmAabBox<PointType>& GetTreeAabb() const { return m_treeAabb; }

	bool IsTreePrepared() const { return m_rootNode != INVALID_NODE_HANDLE; }

	void PrepareTree()
	{
		if (!IsTreePrepared()) {
			YM_IS_TRUE(m_nodes.size() <= INVALID_NODE_HANDLE);
			m_rootNode = ConstructSubTree(
				0, m_nodes.size(), 0, GetTreeAabb().GetMinPoint(), GetTreeAabb().GetMaxPoint()
			);
		}
	}

	template<class VISITOR>
	void VisitTree(VISITOR* pVisitor) const
	{
		YM_IS_TRUE(IsTreePrepared());
		VisitTreeImpl(pVisitor);
	}

	bool FindNearestContent(const PointType& givenPoint, ContentType* pFoundContent, PointType* pFoundPoint) const
	{
		YmKdTreeNearestPointFinder<ContentType, PointType> finder(givenPoint);
		VisitTree(&finder);
		if (finder.IsFoundNearestContent()) {
			if (pFoundContent != nullptr) {
				*pFoundContent = finder.GetNearestContent();
			}
			if (pFoundPoint) {
				*pFoundPoint = ContentPolicy::GetPoint(finder.GetNearestContent());
			}
		}
		return finder.IsFoundNearestContent();
	}
private:
	void InvalidateTree()
	{
		m_rootNode = INVALID_NODE_HANDLE;
	}

	NodeHandle ConstructSubTree(
		size_t nodeIndexBegin, size_t nodeIndexEnd, int splittingAxis, const PointType& boxLowerBound, const PointType& boxUpperBound
	)
	{
		YM_IS_TRUE(nodeIndexBegin <= nodeIndexEnd);
		if (nodeIndexBegin == nodeIndexEnd) {
			return INVALID_NODE_HANDLE;
		}

		size_t midNode = (nodeIndexBegin + nodeIndexEnd) / 2;
		YM_ASSERT(midNode < nodeIndexEnd);
		if (nodeIndexBegin == midNode) {
			YM_ASSERT(midNode + 1 == nodeIndexEnd);
		}
		else {
			std::nth_element(
				m_nodes.begin() + nodeIndexBegin,
				m_nodes.begin() + midNode,
				m_nodes.begin() + nodeIndexEnd,
				NodeComparator(splittingAxis));
		}

		NodeHandle nextNodeIndex = midNode;
		Node& nextNode = m_nodes[nextNodeIndex];
		nextNode.axisIndex = splittingAxis;
		nextNode.threshold = ContentPolicy::GetPoint(nextNode.content)[splittingAxis];

		int nextAxis = ConstructionPolicy::GetNextSplittingAxis(splittingAxis, boxLowerBound, boxUpperBound);

		PointType tempPoint = boxUpperBound;
		tempPoint[splittingAxis] = nextNode.threshold;
		nextNode.smallerNodeIndex = ConstructSubTree(nodeIndexBegin, nextNodeIndex, nextAxis, boxLowerBound, tempPoint);

		tempPoint = boxLowerBound;
		tempPoint[splittingAxis] = nextNode.threshold;
		nextNode.biggerNodeIndex = ConstructSubTree(nextNodeIndex + 1, nodeIndexEnd, nextAxis, tempPoint, boxUpperBound);

		return nextNodeIndex;
	}

	template<class VISITOR>
	void VisitTreeImpl(VISITOR* pVisitor) const
	{
		YM_IS_TRUE(pVisitor != nullptr);
		struct StackFrame {
			NodeHandle currentIndex;
			PointType areaLowerBound;
			PointType areaUpperBound;
		};
		std::vector<StackFrame> stacks;
		stacks.reserve(20);
		stacks.push_back(
			{ m_rootNode, GetTreeAabb().GetMinPoint(), GetTreeAabb().GetMaxPoint() }
		);

		while (!stacks.empty()) {
			StackFrame frame = stacks.back();
			stacks.pop_back();

			const Node& currentNode = m_nodes.at(frame.currentIndex);
			if (!pVisitor->IsVisitThisBox(frame.areaLowerBound, frame.areaUpperBound)) {
				continue;
			}

			bool isContinue = pVisitor->OnVisitContent(currentNode.content, ContentPolicy::GetPoint(currentNode.content));
			if (!isContinue) {
				return;
			}

			YmKdTreeVisitor::Order visitingOrder = pVisitor->DecideVisitingOrder(
				currentNode.axisIndex, currentNode.threshold, frame.areaLowerBound, frame.areaUpperBound
			);

			bool aVisitSmaller[2] = { true, false };	//[0] visit first, [1] visit second
			switch (visitingOrder) {
			case YmKdTreeVisitor::Order::SMALLER_FIRST:
			case YmKdTreeVisitor::Order::DEFAULT:
				break;
			case YmKdTreeVisitor::Order::BIGGER_FIRST:
				aVisitSmaller[0] = false;
				aVisitSmaller[1] = true;
				break;
			case YmKdTreeVisitor::Order::STOP:
				return;
			default:
				YM_THROW_ERROR("Invalid visiting order");
			}

			for (int iChild = 1; 0 <= iChild; --iChild) {
				StackFrame childFrame = frame;
				if (aVisitSmaller[iChild]) {
					childFrame.currentIndex = currentNode.smallerNodeIndex;
					childFrame.areaUpperBound[currentNode.axisIndex] = currentNode.threshold;
				}
				else {
					childFrame.currentIndex = currentNode.biggerNodeIndex;
					childFrame.areaLowerBound[currentNode.axisIndex] = currentNode.threshold;
				}
				if (childFrame.currentIndex == INVALID_NODE_HANDLE) {
					continue;
				}
				stacks.push_back(childFrame);
			}
		}
	}

private:
	std::deque<Node> m_nodes;
	YmAabBox<PointType> m_treeAabb;
	NodeHandle m_rootNode = INVALID_NODE_HANDLE;
};
}
