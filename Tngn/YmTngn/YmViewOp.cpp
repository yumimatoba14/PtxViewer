#include "pch.h"
#include "YmViewOp.h"
#include "YmTngnVectorUtil.h"

using namespace std;
using namespace Ymcpp;

////////////////////////////////////////////////////////////////////////////////

YmViewOp::YmViewOp()
{
	m_eyePoint = YmVectorUtil::Make(0, -3, 0);
	m_eyeDirection = YmVectorUtil::Make(0, 1, 0);
	m_upDirection = YmVectorUtil::Make(0, 0, 1);
	m_verticalDirection = YmVector3d::MakeZero();
	m_orthographicLengthPerDot = 0.01;
	m_isPerspectiveView = true;
}

YmViewOp::~YmViewOp()
{
}

////////////////////////////////////////////////////////////////////////////////

DirectX::XMMATRIX YmViewOp::GetViewMatrix() const
{
	using namespace DirectX;
	XMFLOAT3 eyePoint = YmVectorUtil::StaticCast<XMFLOAT3>(m_eyePoint);
	XMFLOAT3 eyeDirection = YmVectorUtil::StaticCast<XMFLOAT3>(m_eyeDirection);
	XMFLOAT3 upDirection = YmVectorUtil::StaticCast<XMFLOAT3>(m_upDirection);	
	return XMMatrixLookToRH(XMLoadFloat3(&eyePoint), XMLoadFloat3(&eyeDirection), XMLoadFloat3(&upDirection));
}

void YmViewOp::SetVerticalDirection(double zeroTol, const YmVector3d& vertDir)
{
	if (!YmVectorUtil::TryNormalize(zeroTol, vertDir, &m_verticalDirection)) {
		m_verticalDirection = YmVector3d::MakeZero();
	}
}

////////////////////////////////////////////////////////////////////////////////

void YmViewOp::OnMouseWheel(int delta)
{
	if (IsPerspectiveViewMode()) {
		double stepLength = 1;
		GoForward((delta < 0 ? -1 : 1) * stepLength);
	}
	else {
		const double lengthPerDotMin = 1e-12;
		const double coef = 2;
		if (delta < 0) {
			m_orthographicLengthPerDot *= coef;
		}
		else {
			m_orthographicLengthPerDot /= coef;
		}
		if (m_orthographicLengthPerDot < lengthPerDotMin) {
			m_orthographicLengthPerDot = lengthPerDotMin;
		}
	}
	OnChanged();
}

void YmViewOp::OnMouseButtonDown(const YmVector2i& point, MouseStartOption button)
{
	switch (button) {
	case MouseStartOption::L_BUTTON: m_mode = Mode::ROTATE; break;
	case MouseStartOption::R_BUTTON: m_mode = Mode::PAN; break;
	}
}

void YmViewOp::OnMouseButtonUp(const YmVector2i& point, MouseStartOption button)
{
	m_mode = Mode::NONE;
}

void YmViewOp::OnMouseMove(const YmVector2i& point)
{
	YmVector2i diff = point - m_lastMousePoint;
	if (m_mode == Mode::PAN) {
		double velocity = 0.01;
		Pan(-velocity * diff[0], velocity * diff[1]);

		OnChanged();
	}
	else if (m_mode == Mode::ROTATE) {
		constexpr double baseAngleRad = 45 / 180.0 * M_PI * 0.5f / 100;
#if 0
		RotateHorizontally(-diff[0] * baseAngleRad);
#else
		RotateAroundVerticalDirection(-diff[0] * baseAngleRad);
#endif
		RotateVertically(diff[1] * baseAngleRad);

		OnChanged();
	}
	m_lastMousePoint = point;
}

////////////////////////////////////////////////////////////////////////////////

void YmViewOp::GoForward(double length)
{
	m_eyePoint += length * m_eyeDirection;
}

void YmViewOp::Pan(double distanceToRight, double distanceToUp)
{
	m_eyePoint += distanceToRight * GetRightDir() + distanceToUp * m_upDirection;
}

void YmViewOp::RotateHorizontally(double rightAngleRad)
{
	YmVector3d rightDir = GetRightDir();

	double c = cos(rightAngleRad);
	double s = sin(rightAngleRad);
	YmVector3d newDir = c * m_eyeDirection + s * rightDir;

	const double zeroTol = 1e-12;
	m_eyeDirection = YmVectorUtil::Normalize(zeroTol, newDir);
}

void YmViewOp::RotateAroundVerticalDirection(double rightAngleRad)
{
	using namespace YmVectorUtil;
	const double PERP_ANGLE_TOL_COS = 0.5;
	// m_verticalDirection can be zero.
	double upAngleCos = InnerProduct(m_verticalDirection, m_upDirection);
	if (fabs(upAngleCos) < PERP_ANGLE_TOL_COS) {
		RotateHorizontally(rightAngleRad);
		return;
	}

	YmVector3d rightDir = GetRightDir();
	double rightDirAngleCos = InnerProduct(m_verticalDirection, rightDir);
	double eyeDirAngleCos = InnerProduct(m_verticalDirection, m_eyeDirection);
	if (fabs(rightDirAngleCos) > PERP_ANGLE_TOL_COS || fabs(eyeDirAngleCos) > PERP_ANGLE_TOL_COS) {
		RotateHorizontally(rightAngleRad);
		return;
	}

	const double zeroTol = 1e-12;
	YmVector3d refRightDir = Normalize(zeroTol, OuterProduct(m_eyeDirection, m_verticalDirection));
	YmVector3d refDir = OuterProduct(m_verticalDirection, refRightDir);

	double rotAngleRad = rightAngleRad * (upAngleCos < 0 ? -1 : 1);
	float c = (float)cos(rotAngleRad);
	float s = (float)sin(rotAngleRad);
	YmVector3d newRefDir = c * refDir + s * refRightDir;
	YmVector3d newRightDir = c * refRightDir - s * refDir;

	YmVector3d newEyeDir = eyeDirAngleCos * m_verticalDirection + sqrt(1 - eyeDirAngleCos * eyeDirAngleCos) * newRefDir;
	YmVector3d newUpDir = OuterProduct(newRightDir, newEyeDir);
	if (upAngleCos < 0) {
		newUpDir *= -1;
	}

	newEyeDir = Normalize(zeroTol, newEyeDir);
	newUpDir = Normalize(zeroTol, newUpDir);

	m_eyeDirection = newEyeDir;
	m_upDirection = newUpDir;
}

void YmViewOp::RotateVertically(double upAngleRad)
{
	double c = cos(upAngleRad);
	double s = sin(upAngleRad);
	const double zeroTol = 1e-12;
	YmVector3d newEyeDir = YmVectorUtil::Normalize(zeroTol, c * m_eyeDirection + s * m_upDirection);
	YmVector3d newUpDir = YmVectorUtil::Normalize(zeroTol, c * m_upDirection - s * m_eyeDirection);

	m_eyeDirection = newEyeDir;
	m_upDirection = newUpDir;
}

YmVector3d YmViewOp::GetRightDir() const
{
	return YmVectorUtil::OuterProduct(m_eyeDirection, m_upDirection);
}

void YmViewOp::OnChanged() noexcept
{
	YM_NOEXCEPT_BEGIN("YmViewOp::OnChanged");
	for (auto& cb : m_onChangedCallbacks) {
		cb();
	}
	YM_NOEXCEPT_END;
}

////////////////////////////////////////////////////////////////////////////////
