#pragma once

#include "TngnCli.h"
#include "Vector3d.h"

namespace Tngn {
ref class ViewModel;

public ref class ViewEventListener
{
public:
	static ViewEventListener^ GetDefaultInstance();
public:
	ViewEventListener();
	virtual ~ViewEventListener();
	!ViewEventListener();

public:
	void Detach(ViewModel^ viewModel);
	void Attach(ViewModel^ viewModel);
	bool PickingPoint(const Vector3d coord);
	bool PickingNone();
	bool IsUnpickEnabled();

protected:
	virtual void OnDetach(ViewModel^ viewModel) {}
	virtual void OnAttach(ViewModel^ viewModel) {}
	virtual bool OnPickingPoint(const Vector3d coord) { return true; }
	virtual bool OnPickingNone() { return true; }
	virtual bool OnIsUnpickEnabled() { return true; }

private:
	static ViewEventListener^ pDefaultInstance = gcnew ViewEventListener();
};

}
