#include "pch.h"
#include "YmTngnDmMemoryPointList.h"
#include "YmTngnDraw.h"
#include <vector>
#include "YmTngnVectorUtil.h"

using namespace std;
using namespace Ymcpp;
using namespace DirectX;

////////////////////////////////////////////////////////////////////////////////

YmTngnDmMemoryPointList::YmTngnDmMemoryPointList() : m_isUseScannerPoint(false)
{
}

YmTngnDmMemoryPointList::YmTngnDmMemoryPointList(std::vector<YmTngnPointListVertex> dataSource)
	: m_isUseScannerPoint(false), m_dataSource(move(dataSource))
{
}

YmTngnDmMemoryPointList::~YmTngnDmMemoryPointList()
{
}

void YmTngnDmMemoryPointList::SetScannerPosition(const YmVector3d& scannerPos)
{
	m_scannerPosition = scannerPos;
	m_isUseScannerPoint = true;
}

void YmTngnDmMemoryPointList::ResetScannerPosition()
{
	m_isUseScannerPoint = false;
}

void YmTngnDmMemoryPointList::AddPoint(const YmVector3d& position, const YmRgba4b& color)
{
	YmTngnPointListVertex vertex;
	vertex.position = YmVectorUtil::StaticCast<XMFLOAT3>(position);
	vertex.rgba = color.ToUint32();
	m_dataSource.push_back(vertex);
	m_pVertexBuffer = nullptr;
}

void YmTngnDmMemoryPointList::SetupXZRectanglePoints(const YmVector3d& basePos, const YmRgba4b& color)
{
	m_dataSource = CreateXZRectanglePoints(basePos, color);
	m_pVertexBuffer = nullptr;
}

std::vector<YmTngnPickedPoint> YmTngnDmMemoryPointList::FindPickedPoints(YmTngnPickTargetId id) const
{
	vector<YmTngnPickedPoint> points;
	if (GetPointPickTargetIdFirst() != YM_TNGN_PICK_TARGET_NULL) {
		if (GetPointPickTargetIdFirst() <= id && id < GetPointPickTargetIdFirst() + m_dataSource.size()) {
			const auto vtx = m_dataSource[size_t(id - GetPointPickTargetIdFirst())];
			points.push_back(YmTngnPickedPoint{ id, vtx.position, vtx.rgba });
		}
	}
	return points;
}

////////////////////////////////////////////////////////////////////////////////

bool YmTngnDmMemoryPointList::OnSetPickEnabled(bool bEnable)
{
	return bEnable;
}

void YmTngnDmMemoryPointList::OnDraw(YmTngnDraw* pDraw)
{
	if (IsSupportProgressiveMode() && pDraw->IsProgressiveViewFollowingFrame()) {
		return;
	}
	if (!m_pVertexBuffer) {
		m_nVertex = m_dataSource.size();
		if (m_dataSource.empty()) {
			return;
		}
		m_pVertexBuffer = pDraw->CreateVertexBuffer(
			m_dataSource.data(), static_cast<UINT>(m_nVertex), false
		);
	}

	YmTngnPickTargetId firstId = YM_TNGN_PICK_TARGET_NULL;
	if (IsPickEnabled()) {
		firstId = pDraw->MakePickTargetId();
		SetPointPickTargetIdFirst(firstId);
	}

	if (m_isUseScannerPoint) {
		pDraw->DrawPointListWithSingleScannerPosition(
			m_pVertexBuffer, sizeof(YmTngnPointListVertex), m_nVertex, m_scannerPosition, firstId
		);
	}
	else {
		pDraw->DrawPointList(m_pVertexBuffer, sizeof(YmTngnPointListVertex), m_nVertex, firstId);
	}
}

////////////////////////////////////////////////////////////////////////////////

std::vector<YmTngnPointListVertex> YmTngnDmMemoryPointList::CreateXZRectanglePoints(
	const YmVector3d& basePos, const YmRgba4b& color
)
{
	vector<YmTngnPointListVertex> vertices;
	YmTngnPointListVertex vertex;
	vertex.rgba = color.ToUint32();
	const int nX = 100;
	const int nZ = 100;
	double aDelta[3] = { 0.01, 1, 0.01 };
	for (int iX = 0; iX < nX; ++iX) {
		for (int iZ = 0; iZ < nZ; ++iZ) {
			vertex.position = YmVectorUtil::StaticCast<XMFLOAT3>(
				basePos + YmVectorUtil::Make<double>(iX * aDelta[0], 0, iZ * aDelta[2]));
			vertices.push_back(vertex);
		}
	}
	return vertices;
}

////////////////////////////////////////////////////////////////////////////////

YmTngnDmMemoryPointListXZRectangle::YmTngnDmMemoryPointListXZRectangle()
	: YmTngnDmMemoryPointListXZRectangle(YmVector3d::MakeZero(), YmRgba4b(0xFF, 0xFF, 0xFF))
{
}

YmTngnDmMemoryPointListXZRectangle::YmTngnDmMemoryPointListXZRectangle(const YmVector3d& basePos, const YmRgba4b& color)
{
	SetupXZRectanglePoints(basePos, color);
}

////////////////////////////////////////////////////////////////////////////////
