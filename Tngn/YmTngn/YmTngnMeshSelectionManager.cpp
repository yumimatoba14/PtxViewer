#include "pch.h"
#include "YmTngnMeshSelectionManager.h"
#include "YmTngnDmTriangleMesh.h"

using namespace std;
using namespace Ymcpp;

////////////////////////////////////////////////////////////////////////////////

YmTngnMeshSelectionManager::YmTngnMeshSelectionManager()
{
}

YmTngnMeshSelectionManager::~YmTngnMeshSelectionManager()
{
}

////////////////////////////////////////////////////////////////////////////////

void YmTngnMeshSelectionManager::SelectMesh(const YmTngnDmTriangleMeshPtr& pMesh)
{
	YM_IS_TRUE(pMesh);
	m_selectedMeshMap.emplace(pMesh, nullptr);
}

void YmTngnMeshSelectionManager::DeselectMesh(const YmTngnDmTriangleMeshPtr& pMesh)
{
	YM_IS_TRUE(pMesh);
	m_selectedMeshMap.erase(pMesh);
}

void YmTngnMeshSelectionManager::ClearSelection()
{
	m_selectedMeshMap.clear();
}

void YmTngnMeshSelectionManager::Draw(YmTngnDraw* pDraw)
{
	for (auto& meshPair : m_selectedMeshMap) {
		if (!meshPair.second) {
			meshPair.second = meshPair.first->CreateClone();
			meshPair.second->SetColor(YmRgba4b(255, 0, 0));
		}
		meshPair.second->Draw(pDraw);
	}
}

////////////////////////////////////////////////////////////////////////////////
