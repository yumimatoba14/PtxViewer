#include "pch.h"
#include "YmRaycastTarget.h"

using namespace std;
using namespace Ymcpp;

////////////////////////////////////////////////////////////////////////////////

YmRaycastTarget::YmRaycastTarget()
{
}

YmRaycastTarget::~YmRaycastTarget()
{
}

////////////////////////////////////////////////////////////////////////////////

bool YmRaycastTarget::RaycastFrom(
	const YmVector3d& rayOrigin, const YmVector3d& rayDir, double* pRayParam, YmVector3d* pDestinationPoint
)
{
	double rayParam = 0;
	bool isInt = OnRaycastFrom(rayOrigin, rayDir, &rayParam);
	if (!isInt) {
		return false;
	}
	if (pRayParam) {
		*pRayParam = rayParam;
	}
	if (pDestinationPoint) {
		*pDestinationPoint = rayOrigin + rayParam * rayDir;
	}
	return true;
}
