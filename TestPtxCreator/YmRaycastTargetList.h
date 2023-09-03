#pragma once

#include "YmRaycastTarget.h"

namespace Ymcpp {

class YmRaycastTargetList : public YmRaycastTarget
{
public:
	YmRaycastTargetList();
	virtual ~YmRaycastTargetList();

	void Add(const YmRaycastTargetPtr& child) { m_children.push_back(child); }
protected:
	virtual bool OnRaycastFrom(const YmVector3d& rayOrigin, const YmVector3d& rayDir, double* pRayParam);

private:
	double m_coincTol;
	std::vector<YmRaycastTargetPtr> m_children;
};

}
