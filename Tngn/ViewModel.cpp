#include "pch.h"
#include "ViewModel.h"
#include "YmTngn/YmViewOp.h"
#include "YmTngn/YmTngnDmPtxFiles.h"
#include "YmTngn/YmTngnDmMemoryPointList.h"
#include "YmTngn/YmTngnViewConfig.h"
#include "YmBase/YmFilePath.h"
#include <msclr/marshal_cppstd.h>

using namespace System;
using namespace Tngn;
using namespace Ymcpp;
using namespace msclr::interop;

////////////////////////////////////////////////////////////////////////////////

ViewModel::ViewModel(IContainer^ container, System::IntPtr handleWnd)
{
	container->Add(this);
	YmTngnViewConfig config;
	System::String^ baseDir = System::AppDomain::CurrentDomain->BaseDirectory;
	config.ReadIniFile(
		YmFilePath(marshal_as<YmTString>(baseDir)).AppendFileName("Tngn.ini").ToString().c_str()
	);
	m_pImpl = new YmTngnViewModel(config);
	HWND hWnd = reinterpret_cast<HWND>(handleWnd.ToPointer());
	m_pImpl->Setup(hWnd);
	m_pImpl->GetViewOp().SetVerticalDirection(1e-6, YmVectorUtil::Make(0, 0, 1));
#if defined(_DEBUG)
	m_pImpl->SetContent(std::make_unique<YmTngnDmMemoryPointListXZRectangle>());
	m_pImpl->SetSelectedContent(std::make_shared<YmTngnDmMemoryPointListXZRectangle>(
		YmVectorUtil::Make(0, 1, 0), YmRgba4b(0xFF, 0, 0)));
#endif
}

ViewModel::~ViewModel()
{
	if (!isDisposed) {
		Disposed(this, EventArgs::Empty);
		isDisposed = true;
	}
	this->!ViewModel();
}

ViewModel::!ViewModel()
{
	YM_ASSERT(isDisposed);
	delete m_pImpl;
	m_pImpl = nullptr;
}

////////////////////////////////////////////////////////////////////////////////

bool ViewModel::OpenPtxFile(System::String^ ptxFilePath)
{
	YM_NOEXCEPT_BEGIN("ViewModel::OpenPtxFile");
	m_pImpl->PreparePtxFileContent()->ReadPtxFile(marshal_as<YmTString>(ptxFilePath));
	m_pImpl->SetSelectedContent(nullptr);
	return true;
	YM_NOEXCEPT_END;
	return false;
}

bool ViewModel::IsProgressiveViewMode()
{
	return m_pImpl->IsProgressiveViewMode();
}

void ViewModel::SetProgressiveViewMode(bool enableProgressiveView, bool isFollowingFrame)
{
	m_pImpl->SetProgressiveViewMode(enableProgressiveView, isFollowingFrame);
}

bool ViewModel::IsDrawWithScannerPosition()
{
	return m_pImpl->IsDrawWithScannerPosition();
}

void ViewModel::SetDrawWithScannerPosition(bool isUse)
{
	m_pImpl->SetDrawWithScannerPosition(isUse);
}

bool ViewModel::IsPickEnabled()
{
	return m_pImpl->IsPickEnabled();
}

void ViewModel::SetPickEnabled(bool isEnabled)
{
	m_pImpl->SetPickEnabled(isEnabled);
}

////////////////////////////////////////////////////////////////////////////////

void ViewModel::OnSize(System::Drawing::Size viewSize)
{
	YM_IS_TRUE(m_pImpl);
	m_pImpl->ResizeBuffer(YmVectorUtil::Make(viewSize.Width, viewSize.Height));
}

void ViewModel::Draw()
{
	YM_IS_TRUE(m_pImpl);
	m_pImpl->Draw();
}

void ViewModel::OnMouseWheel(System::Windows::Forms::MouseEventArgs^ e)
{
	m_pImpl->GetViewOp().OnMouseWheel(e->Delta);
}

static bool ConvertButtonType(
	System::Windows::Forms::MouseButtons inButton, YmViewOp::MouseStartOption* pOutButton
)
{
	YM_IS_TRUE(pOutButton != nullptr);
	switch (inButton) {
	case System::Windows::Forms::MouseButtons::Left: *pOutButton = YmViewOp::MouseStartOption::L_BUTTON; break;
	case System::Windows::Forms::MouseButtons::Right: *pOutButton = YmViewOp::MouseStartOption::R_BUTTON; break;
	case System::Windows::Forms::MouseButtons::Middle: *pOutButton = YmViewOp::MouseStartOption::M_BUTTON; break;
	default:
		return false;
	}
	return true;
}

static YmVector2i GetLocation(System::Windows::Forms::MouseEventArgs^ e)
{
	return YmVectorUtil::Make(e->Location.X, e->Location.Y);
}

void ViewModel::OnMouseButtonDown(System::Windows::Forms::MouseEventArgs^ e)
{
	YmViewOp::MouseStartOption button;
	if (ConvertButtonType(e->Button, &button)) {
		m_pImpl->GetViewOp().OnMouseButtonDown(GetLocation(e), button);
	}
	if (button == YmViewOp::MouseStartOption::L_BUTTON) {
		isPicking = IsPickEnabled();
	}
}

void ViewModel::OnMouseButtonUp(System::Windows::Forms::MouseEventArgs^ e)
{
	YmViewOp::MouseStartOption button;
	if (ConvertButtonType(e->Button, &button)) {
		m_pImpl->GetViewOp().OnMouseButtonUp(GetLocation(e), button);
	}
	if (button == YmViewOp::MouseStartOption::L_BUTTON && isPicking) {
		auto points = m_pImpl->TryToPickPoint(GetLocation(e));
		if (points.empty()) {
			m_pImpl->PrepareSelectedPointList()->ClearPoint();
		}
		else {
			for (YmTngnPointListVertex& point : points) {
				m_pImpl->PrepareSelectedPointList()->AddPoint(point.position, YmRgba4b(255, 0, 0));
			}
		}
	}
}

void ViewModel::OnMouseMove(System::Windows::Forms::MouseEventArgs^ e)
{
	m_pImpl->GetViewOp().OnMouseMove(GetLocation(e));
	isPicking = false;
}

////////////////////////////////////////////////////////////////////////////////
