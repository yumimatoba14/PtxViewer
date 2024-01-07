#include "pch.h"
#include "YmTngnDmObjFiles.h"
#include "YmTngnModel/YmObjToIndexedTriangleListConverter.h"

using namespace std;
using namespace Ymcpp;
using namespace ATL;

////////////////////////////////////////////////////////////////////////////////

YmTngnDmObjFiles::YmTngnDmObjFiles(const YmTngnViewConfig& config) : m_config(config)
{
}

YmTngnDmObjFiles::~YmTngnDmObjFiles()
{
}

////////////////////////////////////////////////////////////////////////////////

void YmTngnDmObjFiles::ReadObjFile(const YmTString& filePath)
{
	MeshPtr pMeshModel = make_shared<YmTngnDmTriangleMesh>();
	YmObjToIndexedTriangleListConverter converter;
	converter.SetDefaultMaterialColor(m_config.GetRgbaValue(YmTngnViewConfig::DEFAULT_POLYGON_COLOR));
	auto meshes = converter.Convert(CT2A(filePath.c_str()));
	for (const auto& pMesh : meshes) {
		pMeshModel->AddIndexedTriangleList(pMesh);
	}
	m_meshes.push_back(pMeshModel);
}

std::vector<YmTngnDmTriangleMeshPtr> YmTngnDmObjFiles::FindPickedMesh(YmTngnPickTargetId id) const
{
	if (IsPickEnabled() && id != YM_TNGN_PICK_TARGET_NULL) {
		vector<YmTngnDmTriangleMeshPtr> resultMeshes;
		for (auto pMesh : m_meshes) {
			auto triangles = pMesh->FindPickedTriangles(id);
			if (!triangles.empty()) {
				resultMeshes.push_back(pMesh);
			}
		}
		return resultMeshes;
	}
	return vector<YmTngnDmTriangleMeshPtr>();
}

////////////////////////////////////////////////////////////////////////////////

bool YmTngnDmObjFiles::OnSetPickEnabled(bool isEnable)
{
	return isEnable;
}

void YmTngnDmObjFiles::OnDraw(YmTngnDraw* pDraw)
{
	bool isPicking = IsPickEnabled();
	for (auto pMesh : m_meshes) {
		pMesh->SetPickEnabled(isPicking);
		pMesh->Draw(pDraw);
	}
}

////////////////////////////////////////////////////////////////////////////////
