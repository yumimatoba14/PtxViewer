#pragma once

#include "YmRaycastTarget.h"
#include "YmBase/YmAabBox.h"

namespace Ymcpp {

class YmRtRectangle : public YmRaycastTarget
{
public:
	explicit YmRtRectangle(
		const YmVector3d& org, const YmVector3d& dirU, const YmVector3d& dirV,
		double uMin, double uMax, double vMin, double vMax
	);
	virtual ~YmRtRectangle();

protected:
	virtual bool OnRaycastFrom(const YmVector3d& rayOrigin, const YmVector3d& rayDir, double* pRayParam);

private:
	double m_zeroTol;
	YmVector3d m_origin;
	YmVector3d m_dirU;
	YmVector3d m_dirV;
	YmVector3d m_dirN;
	YmAabBox2d m_uvAabb;
};

}
