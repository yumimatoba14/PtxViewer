#include "pch.h"
#include "YmTngnDmExclusiveLodPointList.h"
#include "YmTngnDraw.h"

using namespace std;
using namespace Ymcpp;

////////////////////////////////////////////////////////////////////////////////

YmTngnDmExclusiveLodPointList::YmTngnDmExclusiveLodPointList(YmMemoryMappedFile& imageFile, int64_t imagePos)
	: m_imageFile(imageFile), m_imageByteBegin(imagePos)
{
	PrepareLodTable();
}

YmTngnDmExclusiveLodPointList::~YmTngnDmExclusiveLodPointList()
{
}

////////////////////////////////////////////////////////////////////////////////

void YmTngnDmExclusiveLodPointList::SetScannerPosition(const YmVector3d& scannerPos)
{
	m_scannerPosition = scannerPos;
	m_isUseScannerPosition = true;
}

void YmTngnDmExclusiveLodPointList::ResetScannerPosition()
{
	m_isUseScannerPosition = false;
}

bool YmTngnDmExclusiveLodPointList::FindPointByPickTargetId(YmTngnPickTargetId id, PointType* pFoundVertex) const
{
	if (m_pointPickTargetIdFirst != YM_TNGN_PICK_TARGET_NULL) {
		uint64_t pointNum = m_lodTable.GetPointCount();
		bool isFound = m_pointPickTargetIdFirst <= id && id < m_pointPickTargetIdFirst + pointNum;
		if (isFound && pFoundVertex != nullptr) {
			int64_t vertexIndex = id - m_pointPickTargetIdFirst;
			auto pSrc = m_imageFile.MapView(m_pointByteBegin + vertexIndex * sizeof(PointType), sizeof(PointType));
			*pFoundVertex = pSrc.ToConstArray<PointType>()[0];
		}
		return isFound;
	}
	return false;
}

////////////////////////////////////////////////////////////////////////////////

// Prepare object for drawing.
// This function was used to mark this object re-drawn in the current frame
// but actullay be drawn in the following frame.
void YmTngnDmExclusiveLodPointList::PrepareFirstDraw(YmTngnDraw* pDraw)
{
	if (m_lodTable.GetLevelCount() == 0) {
		PrepareLodTable();
		YM_IS_TRUE(0 < m_lodTable.GetLevelCount());
	}

	m_nextVertex = 0;
	m_drawingVertexEnd = m_lodTable.GetEnoughPointCount(m_drawingPrecision);
}

/// This function can be called sometimes in a frame.
/// PrepareFirstDraw() must have been called beforehand to restart drawing from the begining.
void YmTngnDmExclusiveLodPointList::DrawAfterPreparation(YmTngnDraw* pDraw)
{
	YM_ASSERT(0 < m_lodTable.GetLevelCount());

	if (pDraw->IsProgressiveViewMode()) {
		if (m_nextVertex == 0) {
			m_precisionForFrame = m_lodTable.GetFirstLevelLength();
		}
		else {
			if (m_drawingPrecision < m_precisionForFrame) {
				// Delay drawing the next level and prefer to draw other objects.
				int64_t endVertexInLevel = m_lodTable.GetEnoughPointCount(m_precisionForFrame);
				if (endVertexInLevel <= m_nextVertex) {
#if 1
					// drawing the next level.
					m_precisionForFrame = m_lodTable.GetNextLevelLength(m_precisionForFrame);
#else
					// draw the required level.
					m_precisionForFrame = m_drawingPrecision;
#endif
				}
			}
		}
	}
	else {
		m_precisionForFrame = m_drawingPrecision;
	}
	// m_precisionForFrame has been decided.

	int64_t endVertexInLevel = m_lodTable.GetEnoughPointCount(m_precisionForFrame);
	if (pDraw->IsProgressiveViewMode()) {
		endVertexInLevel = min(endVertexInLevel, m_nextVertex + m_maxPointCountDrawnPerFrame);
	}
#if YM_IS_32BIT_MODULE
	const int64_t nVertexPerCall = UINT_MAX / 24 / sizeof(PointType);	// avoid large array.
#else
	const int64_t nVertexPerCall = UINT_MAX / sizeof(PointType);
#endif
	while (m_nextVertex < endVertexInLevel) {
		int64_t endVertex = min(m_nextVertex + nVertexPerCall, endVertexInLevel);
		size_t nVertex = static_cast<size_t>(endVertex - m_nextVertex);
		UINT dataSize = static_cast<UINT>(nVertex * sizeof(PointType));
		auto pSrc = m_imageFile.MapView(m_pointByteBegin + m_nextVertex * sizeof(PointType), dataSize);

		YmTngnPickTargetId firstId = YM_TNGN_PICK_TARGET_NULL;
		if (IsPickEnabled() && m_pointPickTargetIdFirst != YM_TNGN_PICK_TARGET_NULL) {
			firstId = m_pointPickTargetIdFirst + m_nextVertex;
		}
		if (m_isUseScannerPosition) {
			pDraw->DrawPointListWithSingleScannerPosition(pSrc.ToConstArray<PointType>(), nVertex, m_scannerPosition, firstId);
		}
		else {
			pDraw->DrawPointList(pSrc.ToConstArray<PointType>(), nVertex, firstId);
		}

		m_nextVertex = endVertex;
	}
}

////////////////////////////////////////////////////////////////////////////////

bool YmTngnDmExclusiveLodPointList::OnSetPickEnabled(bool bEnable)
{
	return bEnable && (m_pointPickTargetIdFirst != YM_TNGN_PICK_TARGET_NULL);
}

void YmTngnDmExclusiveLodPointList::OnDraw(YmTngnDraw* pDraw)
{
	if (m_lodTable.GetLevelCount() == 0 || !pDraw->IsProgressiveViewFollowingFrame()) {
		PrepareFirstDraw(pDraw);
	}

	DrawAfterPreparation(pDraw);
}

////////////////////////////////////////////////////////////////////////////////

void YmTngnDmExclusiveLodPointList::PrepareLodTable()
{
	using HeaderType = YmTngnModel::ExclusiveLodPointList::Header;
	auto pSrc = m_imageFile.MapView(m_imageByteBegin, sizeof(HeaderType));
	HeaderType header = pSrc.ToConstArray<HeaderType>()[0];
	if (YmTngnModel::CURRENT_FILE_VERSION < header.version) {
		YM_THROW_ERROR("New file cannot be read.");
	}

	m_lodTable.ReadFrom(m_imageFile, header.lodTablePos);
	m_pointByteBegin = m_imageByteBegin + sizeof(HeaderType);
}

////////////////////////////////////////////////////////////////////////////////
