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

	/// <summary>
	/// Returns whether this object supports progressive view mode.
	/// If it is supported, this object is drawn only once in a drawing.
	/// If not supported, object is drawn evry time when OnDraw() is called.
	/// </summary>
	/// <returns></returns>
	bool IsSupportProgressiveMode() const { return m_isSupportProgressiveMode; }
	void SetSupportProgressiveMode(bool isSupport) { m_isSupportProgressiveMode = isSupport; }

	void SetScannerPosition(const YmVector3d& scannerPos);
	void ResetScannerPosition();
	bool IsUseScannerPosition() const { return m_isUseScannerPoint; }
	YmVector3d GetScannerPosition() const { return m_scannerPosition; }

	void AddPoint(const YmVector3d& position, const YmRgba4b& color);
	void ClearPoint() { m_dataSource.clear(); m_pVertexBuffer = nullptr; }
	size_t GetPointCount() const { return m_dataSource.size(); }

	void SetupXZRectanglePoints(const YmVector3d& basePos, const YmRgba4b& color);

	YmTngnPickTargetId GetPointPickTargetIdFirst() const { return m_pointPickTargetIdFirst; }
	void SetPointPickTargetIdFirst(YmTngnPickTargetId id) { m_pointPickTargetIdFirst = id; }

protected:
	virtual bool OnSetPickEnabled(bool bEnable);
	virtual void OnDraw(YmTngnDraw* pDraw);
	virtual std::vector<YmTngnPointListVertex> OnFindPickedPoints(YmTngnPickTargetId id);

private:
	std::vector<YmTngnPointListVertex> CreateXZRectanglePoints(const YmVector3d& basePos, const YmRgba4b& color);

private:
	D3DBufferPtr m_pVertexBuffer;
	size_t m_nVertex;
	YmTngnPickTargetId m_pointPickTargetIdFirst = YM_TNGN_PICK_TARGET_NULL;
	bool m_isSupportProgressiveMode = true;
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
