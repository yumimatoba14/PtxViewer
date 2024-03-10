#include "pch.h"
#include "ViewEventListener.h"

using namespace System;
using namespace Tngn;

#define FUNC_BEGIN() try {
#define FUNC_END() } catch(System::Exception^) {}

////////////////////////////////////////////////////////////////////////////////

ViewEventListener^ ViewEventListener::GetDefaultInstance()
{
	return pDefaultInstance;
}


ViewEventListener::ViewEventListener()
{
}

ViewEventListener::~ViewEventListener()
{
	this->!ViewEventListener();
}

ViewEventListener::!ViewEventListener()
{
	// delete unmanaged resource.
}

////////////////////////////////////////////////////////////////////////////////

void ViewEventListener::Detach(ViewModel^ viewModel)
{
	FUNC_BEGIN();
	OnDetach(viewModel);
	FUNC_END();
}

void ViewEventListener::Attach(ViewModel^ viewModel)
{
	FUNC_BEGIN();
	OnAttach(viewModel);
	FUNC_END();
}

bool ViewEventListener::PickingPoint(const Vector3d coord)
{
	FUNC_BEGIN();
	return OnPickingPoint(coord);
	FUNC_END();
	return true;
}

bool ViewEventListener::PickingNone()
{
	FUNC_BEGIN();
	return OnPickingNone();
	FUNC_END();
	return true;
}

bool ViewEventListener::IsUnpickEnabled()
{
	FUNC_BEGIN();
	return OnIsUnpickEnabled();
	FUNC_END();
	return true;
}

////////////////////////////////////////////////////////////////////////////////
