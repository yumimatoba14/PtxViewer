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

////////////////////////////////////////////////////////////////////////////////

void YmTngnDmObjFiles::OnDraw(YmTngnDraw* pDraw)
{
	for (auto pMesh : m_meshes) {
		pMesh->Draw(pDraw);
	}
}

////////////////////////////////////////////////////////////////////////////////
