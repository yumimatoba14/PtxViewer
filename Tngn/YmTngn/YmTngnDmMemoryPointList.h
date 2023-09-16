#pragma once

#include "YmTngnDrawingModel.h"
#include "YmTngnDraw.h"
#include "YmBase/YmVector.h"
#include "YmTngnModel/YmRgba.h"
#include <vector>

namespace Ymcpp {

class YmTngnDmMemoryPointListXZRectangle;

class YmTngnDmMemoryPointList : public YmTngnDrawingModel
{
public:
	YmTngnDmMemoryPointList();
	explicit YmTngnDmMemoryPointList(std::vector<YmTngnPointListVertex> dataSource);
	virtual ~YmTngnDmMemoryPointList();

	void SetScannerPosition(const YmVector3d& scannerPos);
	void ResetScannerPosition();
	bool IsUseScannerPosition() const { return m_isUseScannerPoint; }
	YmVector3d GetScannerPosition() const { return m_scannerPosition; }

	void AddPoint(const YmVector3d& position, const YmRgba4b& color);
	void ClearPoint() { m_dataSource.clear(); m_pVertexBuffer = nullptr; }

	void SetupXZRectanglePoints(const YmVector3d& basePos, const YmRgba4b& color);

protected:
	virtual void OnDraw(YmTngnDraw* pDraw);

private:
	std::vector<YmTngnPointListVertex> CreateXZRectanglePoints(const YmVector3d& basePos, const YmRgba4b& color);

private:
	D3DBufferPtr m_pVertexBuffer;
	size_t m_nVertex;
	bool m_isUseScannerPoint;
	YmVector3d m_scannerPosition;
	std::vector<YmTngnPointListVertex> m_dataSource;
};

////////////////////////////////////////////////////////////////////////////////

class YmTngnDmMemoryPointListXZRectangle : public YmTngnDmMemoryPointList
{
public:
	YmTngnDmMemoryPointListXZRectangle();
	YmTngnDmMemoryPointListXZRectangle(const YmVector3d& basePos, const YmRgba4b& color);
};

}
