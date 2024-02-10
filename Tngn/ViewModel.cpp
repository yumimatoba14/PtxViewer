#include "pch.h"
#include "ViewModel.h"
#include "YmTngn/YmViewOp.h"
#include "YmTngn/YmTngnDmObjFiles.h"
#include "YmTngn/YmTngnDmPtxFiles.h"
#include "YmTngn/YmTngnDmMemoryPointList.h"
#include "YmTngn/YmTngnDmTriangleMesh.h"
#include "YmTngn/YmTngnMeshSelectionManager.h"
#include "YmTngn/YmTngnViewConfig.h"
#include "YmBase/YmFilePath.h"
#include <msclr/marshal_cppstd.h>
#include <wincodec.h>

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
	eventListener = ViewEventListener::GetDefaultInstance();
	m_pImpl = new YmTngnViewModel(config);
	HWND hWnd = reinterpret_cast<HWND>(handleWnd.ToPointer());
	m_pImpl->Setup(hWnd);
	m_pImpl->GetViewOp().SetVerticalDirection(1e-6, YmVectorUtil::Make(0, 0, 1));
#if defined(_DEBUG)
	m_pImpl->SetContent(std::make_unique<YmTngnDmMemoryPointListXZRectangle>());
	m_pImpl->SetContent(YmTngnDmTriangleMesh::MakeSampleData(YmVectorUtil::Make(2, 1, 0)));
	m_pImpl->SetContent(YmTngnDmTriangleMesh::MakeSampleCylinderData(
		0.01, YmVectorUtil::Make(0, 0, 0), YmVectorUtil::Make(0, 0, 1), 2, 1, YmRgba4b(0, 0, 255), true));
	auto pSelPointList = std::make_shared<YmTngnDmMemoryPointListXZRectangle>(
		YmVectorUtil::Make(0, 1, 0), YmRgba4b(0xFF, 0, 0));
	pSelPointList->SetSupportProgressiveMode(false);
	m_pImpl->SetSelectedContent(pSelPointList);
	m_pImpl->AddLengthDimension(YmVectorUtil::Make(0, 0, 0), YmVectorUtil::Make(1, 0, 1));
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

void ViewModel::SetViewEventListener(ViewEventListener^ listener)
{
	if (listener == nullptr) {
		listener = ViewEventListener::GetDefaultInstance();
	}

	eventListener->Detach(this);
	eventListener = listener;
	eventListener->Attach(this);
}

bool ViewModel::IsViewOrthographic()
{
	return !m_pImpl->IsViewPerspective();
}

void ViewModel::SetViewOrthographic(bool isOrtho)
{
	m_pImpl->SetViewPerspective(!isOrtho);
}

bool ViewModel::OpenPtxFile(System::String^ ptxFilePath)
{
	YM_NOEXCEPT_BEGIN("ViewModel::OpenPtxFile");
	m_pImpl->PreparePtxFileContent()->ReadPtxFile(marshal_as<YmTString>(ptxFilePath));
	m_pImpl->SetSelectedContent(nullptr);
	return true;
	YM_NOEXCEPT_END;
	return false;
}

bool ViewModel::OpenObjFile(System::String^ objFilePath)
{
	YM_NOEXCEPT_BEGIN("ViewModel::OpenObjFile");
	m_pImpl->PrepareObjFileContent()->ReadObjFile(marshal_as<YmTString>(objFilePath));
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

void ViewModel::SaveViewToFile(System::String^ imageFilePath)
{
	YmFilePath filePath = marshal_as<YmTString>(imageFilePath);
	std::string fileExt = filePath.GetExtension();

	GUID fileType = GUID_ContainerFormatPng;
	if (_stricmp(fileExt.c_str(), "png") == 0) {
		fileType = GUID_ContainerFormatPng;
	}
	else if (_stricmp(fileExt.c_str(), "jpeg") == 0) {
		fileType = GUID_ContainerFormatJpeg;
	}
	else if (_stricmp(fileExt.c_str(), "jpg") == 0) {
		fileType = GUID_ContainerFormatJpeg;
	}
	else if (_stricmp(fileExt.c_str(), "bmp") == 0) {
		fileType = GUID_ContainerFormatBmp;
	}

	bool isOk = m_pImpl->SaveViewToFile(fileType, filePath.ToTString().c_str());
	if (!isOk) {
		throw gcnew System::InvalidOperationException(marshal_as<System::String^>("Failed to save image."));
	}
}

static YmVector3d ConvertToYmVector3d(Vector3d point)
{
	YmVector3d out;
	out[0] = point.X;
	out[1] = point.Y;
	out[2] = point.Z;
	return out;
}

void ViewModel::AddLengthDimension(Vector3d point0, Vector3d point1)
{
	m_pImpl->AddLengthDimension(ConvertToYmVector3d(point0), ConvertToYmVector3d(point1));
}

void ViewModel::ClearLengthDimension()
{
	m_pImpl->ClearLengthDimension();
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

static Vector3d MakeVector(const YmVector3d& in)
{
	return Vector3d(in[0], in[1], in[2]);
}

void ViewModel::OnMouseButtonUp(System::Windows::Forms::MouseEventArgs^ e)
{
	YmViewOp::MouseStartOption button;
	if (ConvertButtonType(e->Button, &button)) {
		m_pImpl->GetViewOp().OnMouseButtonUp(GetLocation(e), button);
	}
	if (button == YmViewOp::MouseStartOption::L_BUTTON && isPicking) {
		YmTngnPickTargetId pickedId = m_pImpl->TryToPickAsId(GetLocation(e));
		if (pickedId == YM_TNGN_PICK_TARGET_NULL) {
			if (eventListener->PickingNone()) {
				m_pImpl->PrepareSelectedPointList()->ClearPoint();
			}
			m_pImpl->PrepareMeshSelectionManager()->ClearSelection();
		}
		else {
			auto points = m_pImpl->GetPickedPoint(pickedId);
			for (YmTngnPickedPoint& point : points) {
				bool isAdd = eventListener->PickingPoint(MakeVector(point.positionInModel));
				if (isAdd) {
					m_pImpl->PrepareSelectedPointList()->AddPoint(point.positionInModel, YmRgba4b(255, 0, 0));
				}
			}
			auto meshes = m_pImpl->GetPickedTriangleMesh(pickedId);
			for (auto& pMesh : meshes) {
				m_pImpl->PrepareMeshSelectionManager()->SelectMesh(pMesh);
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
