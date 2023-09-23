#pragma once

#include "TngnCli.h"
#include "YmBase/YmVector.h"
#include "YmTngn/YmTngnViewModel.h"

namespace Tngn {
using namespace System::ComponentModel;

public ref class ViewModel : public IComponent
{
public:
	explicit ViewModel(IContainer^ container, System::IntPtr handleWnd);
	virtual ~ViewModel();
	!ViewModel();

	property ISite^ Site {
		virtual ISite^ get() { return site; }
		virtual void set(ISite^ s) { site = s; }
	}

	virtual event System::EventHandler^ Disposed;

	bool OpenPtxFile(System::String^ ptxFilePath);
	bool IsProgressiveViewMode();
	void SetProgressiveViewMode(bool enableProgressiveView, bool isFollowingFrame);
	bool IsViewContentUpdated() { return m_pImpl->IsViewContentUpdated(); }
public:
	void OnSize(System::Drawing::Size viewSize);
	void Draw();
	bool IsNeedDraw() { return m_pImpl->IsNeedDraw(); }

	void OnMouseWheel(System::Windows::Forms::MouseEventArgs^ e);
	void OnMouseButtonDown(System::Windows::Forms::MouseEventArgs^ e);
	void OnMouseButtonUp(System::Windows::Forms::MouseEventArgs^ e);
	void OnMouseMove(System::Windows::Forms::MouseEventArgs^ e);

private:
	bool isDisposed = false;
	ISite^ site = nullptr;
	Ymcpp::YmTngnViewModel* m_pImpl = nullptr;
	bool isPicking = false;
};

}
