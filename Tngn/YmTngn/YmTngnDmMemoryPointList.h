#pragma once

#include "YmTngnDrawingModel.h"
#include "YmBase/YmVector.h"
#include <vector>

namespace Ymcpp {

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

protected:
	virtual void OnDraw(YmTngnDraw* pDraw);

private:
	std::vector<YmTngnPointListVertex> CreateSampleDataSource();

private:
	D3DBufferPtr m_pVertexBuffer;
	size_t m_nVertex;
	bool m_isUseScannerPoint;
	YmVector3d m_scannerPosition;
	std::vector<YmTngnPointListVertex> m_dataSource;
};

}
