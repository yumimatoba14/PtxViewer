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

////////////////////////////////////////////////////////////////////////////////

void YmTngnDmMemoryPointList::OnDraw(YmTngnDraw* pDraw)
{
	if (!m_pVertexBuffer) {
		if (m_dataSource.empty()) {
			m_dataSource = CreateSampleDataSource();
		}

		m_nVertex = m_dataSource.size();
		m_pVertexBuffer = pDraw->CreateVertexBuffer(
			m_dataSource.data(), static_cast<UINT>(m_nVertex), false
		);
	}

	if (m_isUseScannerPoint) {
		pDraw->DrawPointListWithSingleScannerPosition(
			m_pVertexBuffer, sizeof(YmTngnPointListVertex), m_nVertex, m_scannerPosition
		);
	}
	else {
		pDraw->DrawPointList(m_pVertexBuffer, sizeof(YmTngnPointListVertex), m_nVertex);
	}
}

////////////////////////////////////////////////////////////////////////////////

std::vector<YmTngnPointListVertex> YmTngnDmMemoryPointList::CreateSampleDataSource()
{
	vector<YmTngnPointListVertex> vertices;
	YmTngnPointListVertex vertex;
	vertex.rgba = 0xFFFFFF;
	const int nX = 100;
	const int nZ = 100;
	double aDelta[3] = { 0.01, 1, 0.01 };
	for (int iX = 0; iX < nX; ++iX) {
		for (int iZ = 0; iZ < nZ; ++iZ) {
			vertex.position = YmVectorUtil::StaticCast<XMFLOAT3>(YmVectorUtil::Make<double>(iX * aDelta[0], 0, iZ * aDelta[2]));
			vertices.push_back(vertex);
		}
	}
	return vertices;
}

////////////////////////////////////////////////////////////////////////////////
