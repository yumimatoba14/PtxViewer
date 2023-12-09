#pragma once

#include "YmTngnDrawingModel.h"
#include "YmTngnDmExclusiveLodPointList.h"
#include "YmBase/YmAabBox.h"
#include "YmBase/YmWin32FileBuf.h"

namespace Ymcpp {

/// <summary>
/// Container drawing model class which contains some point block.
/// A point block is implemented by YmTngnDmExclusiveLodPointList.
/// </summary>
class YmTngnDmPointBlockList : public YmTngnDrawingModel
{
public:
	using PointBlockType = YmTngnDmExclusiveLodPointList;
	struct InstanceData {
		DirectX::XMFLOAT4X4 localToGlobalMatrix;
		YmAabBox3d localAabb;	// AABB in pPointBlock's local coordinate system.
		std::shared_ptr<PointBlockType> pPointBlock;
	};
public:
	YmTngnDmPointBlockList();
	virtual ~YmTngnDmPointBlockList();

public:
	void SetScannerPosition(const YmVector3d& scannerPos);
	bool IsUseScannerPosition() const;
	void SetUseScannerPosition(bool isUse);

	int64_t GetMaxDrawnPointCountPerFrame() const { return m_maxDrawnPointCountPerFrame; }
	void SetMaxDrawnPointCountPerFrame(int64_t numPoint) { m_maxDrawnPointCountPerFrame = numPoint; }

	double GetScannerDistanceForPointBlockWithoutScanerPos() const { return m_scannerDistanceForPointBlockWithoutScannerPos; }
	void SetScannerDistanceForPointBlockWithoutScanerPos(double dist) { m_scannerDistanceForPointBlockWithoutScannerPos = dist; }

	const std::vector<InstanceData>& GetInstanceList() const { return m_instanceList; }
	void ClearInstance() { m_instanceList.clear(); }
	void AddInstance(InstanceData instance);
	void AddInstances(const YmTngnDmPointBlockList& sourceInstances);
	void ReserveInstanceList(size_t capacity) { m_instanceList.reserve(capacity); }

protected:
	virtual bool OnSetPickEnabled(bool bEnable);
	virtual void OnDraw(YmTngnDraw* pDraw);
	virtual std::vector<YmTngnPointListVertex> OnFindPickedPoints(YmTngnPickTargetId id);

private:
	void UpdateDrawnInstances(YmTngnDraw* pDraw);
	void DrawInstancesInProgressiveMode(
		YmTngnDraw* pDraw, int64_t maxDrawnPointCountPerFrame
	);

private:
	std::vector<InstanceData> m_instanceList;
	std::vector<size_t> m_drawnInstanceIndices;
	int64_t m_maxDrawnPointCountPerFrame = 1 << 20;
	double m_scannerDistanceForPointBlockWithoutScannerPos = 0;
};

}
