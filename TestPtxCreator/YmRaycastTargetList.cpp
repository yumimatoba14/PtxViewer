#include "pch.h"
#include "YmRaycastTargetList.h"

using namespace std;
using namespace Ymcpp;

////////////////////////////////////////////////////////////////////////////////

YmRaycastTargetList::YmRaycastTargetList()
{
}

YmRaycastTargetList::~YmRaycastTargetList()
{
}

////////////////////////////////////////////////////////////////////////////////

bool YmRaycastTargetList::OnRaycastFrom(const YmVector3d& rayOrigin, const YmVector3d& rayDir, double* pRayParam)
{
	bool isFound = false;
	double foundParam = DBL_MAX;
	auto itEnd = m_children.end();
	for (auto it = m_children.begin(); it != itEnd; ++it) {
		double rayParam = 0;
		bool isInt = (*it)->RaycastFrom(rayOrigin, rayDir, &rayParam, nullptr);
		if (!isInt || rayParam <= 0) {
			continue;
		}
		if (rayParam < foundParam) {
			foundParam = rayParam;
			isFound = true;
		}
	}
	if (isFound) {
		*pRayParam = foundParam;
	}
	return isFound;
}

////////////////////////////////////////////////////////////////////////////////
