#include "pch.h"
#include "ViewModel.h"
#include "YmTngn/YmViewOp.h"
#include "YmTngn/YmTngnDmPtxFiles.h"
#include <msclr/marshal_cppstd.h>
#include "YmTngn/YmTngnDmMemoryPointList.h"

using namespace System;
using namespace Tngn;
using namespace Ymcpp;

////////////////////////////////////////////////////////////////////////////////

ViewModel::ViewModel(IContainer^ container, System::IntPtr handleWnd)
{
	container->Add(this);
	m_pImpl = new YmTngnViewModel();
	HWND hWnd = reinterpret_cast<HWND>(handleWnd.ToPointer());
	m_pImpl->Setup(hWnd);
	m_pImpl->GetViewOp().SetVerticalDirection(1e-6, YmVectorUtil::Make(0, 0, 1));
#if defined(_DEBUG)
	m_pImpl->SetContent(std::make_unique<YmTngnDmMemoryPointList>());
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
	using namespace msclr::interop;
	m_pImpl->PreparePtxFileContent()->ReadPtxFile(marshal_as<YmTString>(ptxFilePath));
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
}

void ViewModel::OnMouseButtonUp(System::Windows::Forms::MouseEventArgs^ e)
{
	YmViewOp::MouseStartOption button;
	if (ConvertButtonType(e->Button, &button)) {
		m_pImpl->GetViewOp().OnMouseButtonUp(GetLocation(e), button);
	}
}

void ViewModel::OnMouseMove(System::Windows::Forms::MouseEventArgs^ e)
{
	m_pImpl->GetViewOp().OnMouseMove(GetLocation(e));
}

////////////////////////////////////////////////////////////////////////////////