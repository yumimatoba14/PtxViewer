#pragma once

#include "YmTngnFwd.h"
#include "YmBase/YmVector.h"
#include <functional>
#include <vector>

namespace Ymcpp {

class YmViewOp
{
public:
	enum class MouseStartOption {
		L_BUTTON = 0x01,
		R_BUTTON = 0x02,
		M_BUTTON = 0x04,
	};
	enum class Mode {
		NONE,
		PAN,
		ROTATE
	};

public:
	YmViewOp();
	virtual ~YmViewOp();

	YmViewOp(const YmViewOp& other) = delete;
	YmViewOp(YmViewOp&& other) noexcept = delete;

	YmViewOp& operator = (const YmViewOp& other) = delete;
	YmViewOp& operator = (YmViewOp&& other) noexcept = delete;

	DirectX::XMMATRIX GetViewMatrix() const;
	void SetVerticalDirection(double zeroTol, const YmVector3d& vertDir);
	bool IsMoving() const { return m_mode != Mode::NONE; }

	double GetOrthographicLengthPerDot() const { return m_orthographicLengthPerDot; }
	void SetOrthographicLengthPerDot(double length) { m_orthographicLengthPerDot = length; OnChanged(); }

	bool IsPerspectiveViewMode() const { return m_isPerspectiveView; }
	void SetPerspectiveViewMode(bool isPerspective) { m_isPerspectiveView = isPerspective; OnChanged(); }

	template<class CB>
	void AddOnChangedCallback(CB&& cb) { m_onChangedCallbacks.push_back(std::forward<CB>(cb)); }
public:
	void OnMouseWheel(int delta);
	void OnMouseButtonDown(const YmVector2i& point, MouseStartOption button);
	void OnMouseButtonUp(const YmVector2i& point, MouseStartOption button);
	void OnMouseMove(const YmVector2i& point);

private:
	void GoForward(double length);
	void Pan(double distanceToRight, double distanceToUp);
	void RotateHorizontally(double rightAngleRad);
	void RotateAroundVerticalDirection(double rightAngleRad);
	void RotateVertically(double upAngleRad);

	YmVector3d GetRightDir() const;
	void OnChanged() noexcept;
private:
	Mode m_mode = Mode::NONE;
	YmVector3d m_eyePoint;
	YmVector3d m_eyeDirection;
	YmVector3d m_upDirection;
	YmVector3d m_verticalDirection;
	YmVector2i m_lastMousePoint;
	double m_orthographicLengthPerDot;
	bool m_isPerspectiveView;
	std::vector<std::function<void()>> m_onChangedCallbacks;
};

}
