#pragma once

#include "YmTngnFwd.h"
#include <map>

namespace Ymcpp {

class YmTngnMeshSelectionManager
{
public:
	YmTngnMeshSelectionManager();
	virtual ~YmTngnMeshSelectionManager();

	YmTngnMeshSelectionManager(const YmTngnMeshSelectionManager& other) = delete;
	YmTngnMeshSelectionManager(YmTngnMeshSelectionManager&& other) noexcept = delete;
	YmTngnMeshSelectionManager& operator = (const YmTngnMeshSelectionManager& other) = delete;
	YmTngnMeshSelectionManager& operator = (YmTngnMeshSelectionManager&& other) noexcept = delete;

	bool IsEmpty() const { return m_selectedMeshMap.empty(); }

	void SelectMesh(const YmTngnDmTriangleMeshPtr& pMesh);
	bool IsSelected(const YmTngnDmTriangleMeshPtr& pMesh) const {
		return m_selectedMeshMap.find(pMesh) != m_selectedMeshMap.end();
	}
	void ClearSelection();

	void Draw(YmTngnDraw* pDraw);
private:
	std::map<YmTngnDmTriangleMeshPtr, YmTngnDmTriangleMeshPtr> m_selectedMeshMap;	// map original to dipslay object.
};

}
