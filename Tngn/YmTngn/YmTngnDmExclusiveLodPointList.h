#pragma once

#include "YmTngnFwd.h"
#include "YmTngnDrawingModel.h"
#include "YmBase/YmMemoryMappedFile.h"
#include "YmTngnModel/YmTngnExclusiveLodPointListLodTable.h"

namespace Ymcpp {

/// <summary>
/// This class doesn't own memory mapped instance itself. It should be kept by someone else.
/// </summary>
class YmTngnDmExclusiveLodPointList : public YmTngnDrawingModel
{
public:
	using PointType = YmTngnPointListVertex;
public:
	explicit YmTngnDmExclusiveLodPointList(YmMemoryMappedFile& imageFile, int64_t imagePos);
	virtual ~YmTngnDmExclusiveLodPointList();

	void SetScannerPosition(const YmVector3d& scannerPos);
	void ResetScannerPosition();
	bool IsUseScannerPosition() const { return m_isUseScannerPosition; }
	YmVector3d GetScannerPosition() const { return m_scannerPosition; }

	void SetDrawingPrecision(double length) { m_drawingPrecision = length; }
	void SetMaxPointCountDrawnPerFrame(int64_t nPoint) { m_maxPointCountDrawnPerFrame = nPoint; }
	//void SetPointSelectionTargetIdFirst(D3DSelectionTargetId id) { m_pointStIdFirst = id; }

	//bool FindPointBySelectionTargetId(D3DSelectionTargetId id, Vertex* pFoundVertex) const;

	void PrepareFirstDraw(YmTngnDraw* pDraw);
	bool IsDrawingEnded() const { return m_drawingVertexEnd <= m_nextVertex; }

	void DrawAfterPreparation(YmTngnDraw* pDraw);

protected:
	virtual void OnDraw(YmTngnDraw* pDraw);

private:
	YmMemoryMappedFile& m_imageFile;
	int64_t m_imageByteBegin = 0;
	double m_drawingPrecision = 0;
	int64_t m_maxPointCountDrawnPerFrame = 1 << 20;	/// used only in case of progressive mode.
	//D3DSelectionTargetId m_pointStIdFirst = D3D_SELECTION_TARGET_NULL;

	// point list data initialized in OnDrawTo() once.
private:
	YmTngnExclusiveLodPointListLodTable m_lodTable;
	int64_t m_pointByteBegin = 0;

	// Work variables used in PrepareFirstDraw() and DrawAfterPreparation().
private:
	double m_precisionForFrame = 0;
	int64_t m_drawingVertexEnd = 0;
	int64_t m_nextVertex = 0;
	YmVector3d m_scannerPosition;
	bool m_isUseScannerPosition = false;
};

using YmTngnDmExclusiveLodPointListPtr = std::shared_ptr<YmTngnDmExclusiveLodPointList>;

}
