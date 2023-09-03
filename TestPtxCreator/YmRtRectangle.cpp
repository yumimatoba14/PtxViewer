#include "pch.h"
#include "YmRtRectangle.h"

using namespace std;
using namespace Ymcpp;

////////////////////////////////////////////////////////////////////////////////

YmRtRectangle::YmRtRectangle(
	const YmVector3d& org, const YmVector3d& dirU, const YmVector3d& dirV,
	double uMin, double uMax, double vMin, double vMax
) : m_zeroTol(1e-12), m_origin(org)
{
	m_dirU = YmVectorUtil::Normalize(m_zeroTol, dirU);
	m_dirN = YmVectorUtil::Normalize(m_zeroTol, YmVectorUtil::OuterProduct(m_dirU, dirV));
	m_dirV = YmVectorUtil::OuterProduct(m_dirN, m_dirU);

	m_uvAabb.Extend(YmVectorUtil::Make(uMin, vMin));
	m_uvAabb.Extend(YmVectorUtil::Make(uMax, vMax));
}

YmRtRectangle::~YmRtRectangle()
{
}

////////////////////////////////////////////////////////////////////////////////

bool YmRtRectangle::OnRaycastFrom(const YmVector3d& rayOrigin, const YmVector3d& rayDir, double* pRayParam)
{
	YM_ASSERT(pRayParam != nullptr);

	// Solve the equation
	// (P - m_origin).Dot(m_dirN) == 0
	// where
	// P = rayOrigin + rayParam * rayDir
	// .
	double raySlope = rayDir.Dot(m_dirN);
	if (fabs(raySlope) < m_zeroTol) {
		return false;
	}

	double rayHeight = (m_origin - rayOrigin).Dot(m_dirN);
	double rayParam = rayHeight / raySlope;
	if (rayParam < -m_zeroTol) {
		return false;
	}

	YmVector3d footPoint = rayOrigin + rayParam * rayDir;
	YmVector3d diff = footPoint - m_origin;
	double u = diff.Dot(m_dirU);
	double v = diff.Dot(m_dirV);
	if (u < m_uvAabb.GetMinPoint()[0] || m_uvAabb.GetMaxPoint()[0] < u) {
		return false;
	}
	if (v < m_uvAabb.GetMinPoint()[1] || m_uvAabb.GetMaxPoint()[1] < v) {
		return false;
	}
	*pRayParam = rayParam;
	return true;
}

////////////////////////////////////////////////////////////////////////////////
