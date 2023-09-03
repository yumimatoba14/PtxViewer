#pragma once

#include "YmBase/YmBaseFwd.h"

namespace Ymcpp {

class YmRaycastTarget
{
public:
	YmRaycastTarget();
	virtual ~YmRaycastTarget();

	YmRaycastTarget(const YmRaycastTarget& other) = delete;
	YmRaycastTarget(YmRaycastTarget&& other) noexcept = delete;

	YmRaycastTarget& operator = (const YmRaycastTarget& other) = delete;
	YmRaycastTarget& operator = (YmRaycastTarget&& other) noexcept = delete;

public:
	bool RaycastFrom(const YmVector3d& rayOrigin, const YmVector3d& rayDir, double* pRayParam, YmVector3d* pDestinationPoint);

protected:
	virtual bool OnRaycastFrom(const YmVector3d& rayOrigin, const YmVector3d& rayDir, double* pRayParam) = 0;
};

using YmRaycastTargetPtr = std::shared_ptr<YmRaycastTarget>;

}
