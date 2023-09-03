#pragma once

#include "YmTngnDrawingModel.h"
#include "YmTngnDmExclusiveLodPointList.h"
#include "YmBase/YmAabBox.h"
#include "YmBase/YmWin32FileBuf.h"

namespace Ymcpp {

class YmTngnDmPointBlockList : public YmTngnDrawingModel
{
public:
	struct InstanceData {
		DirectX::XMFLOAT4X4 localToGlobalMatrix;
		YmAabBox3d aabb;	// AABB in model space.
		std::unique_ptr<YmTngnDmExclusiveLodPointList> pPointBlock;
	};
public:
	explicit YmTngnDmPointBlockList(std::unique_ptr<YmWin32FileBuf> pBaseFile);
	virtual ~YmTngnDmPointBlockList();

public:
	void SetScannerPosition(const YmVector3d& scannerPos);
	void PrepareBlockData();
	const std::vector<InstanceData>& GetIntanceList() const { return m_instanceList; }

protected:
	virtual void OnDraw(YmTngnDraw* pDraw);

private:
	void UpdateDrawnInstances(YmTngnDraw* pDraw);
	void DrawInstancesInProgressiveMode(
		YmTngnDraw* pDraw, int64_t maxDrawnPointCountPerFrame
	);

private:
	std::unique_ptr<YmWin32FileBuf> m_pBaseFile;
	YmMemoryMappedFile m_mmFile;
	std::vector<InstanceData> m_instanceList;
	std::vector<size_t> m_drawnInstanceIndices;
};

}
