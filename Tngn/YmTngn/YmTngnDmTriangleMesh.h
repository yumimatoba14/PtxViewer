#pragma once

#include "YmTngn/YmTngnDrawingModel.h"
#include "YmTngnDrawableObject.h"
#include "YmTngnModel/YmRgba.h"

namespace Ymcpp {

class YmTngnDmTriangleMesh : public YmTngnDrawingModel
{
public:
	using VertexType = YmTngnTriangleVertex;
	using IndexType = UINT;

	class IndexedTriangleList : public YmTngnDrawableObject
	{
	public:
		IndexedTriangleList(YmTngnIndexedTriangleListPtr pModel);

		YmTngnIndexedTriangleListPtr GetModel() const { return m_pModel; }
		YmRgba4b GetColor() const { return m_color; }
		void SetColor(YmRgba4b color) { m_color = color; ClearData(); }
		bool IsTransparent() const { return m_color.GetA() != 255; }
		void Draw(YmTngnDraw* pDraw);
		void PrepareAabb();
		YmTngnPickTargetId GetPickTargetId() const { return m_pickTargetId; }
		void SetPickTargetId(YmTngnPickTargetId id) { m_pickTargetId = id; }

		void ClearData() { m_pVertexBuffer = nullptr; m_pIndexBuffer = nullptr; m_nIndex = 0; }
		std::shared_ptr<IndexedTriangleList> CreateClone() const;
	protected:
		void virtual OnDraw(YmTngnDraw* pDraw) { Draw(pDraw); }
	private:
		void PrepareData(YmTngnDraw* pDraw);
	private:
		YmTngnIndexedTriangleListPtr m_pModel;
		YmRgba4b m_color;
		D3DBufferPtr m_pVertexBuffer;
		D3DBufferPtr m_pIndexBuffer;
		size_t m_nIndex = 0;
		YmTngnPickTargetId m_pickTargetId = YM_TNGN_PICK_TARGET_NULL;
	};
	using IndexedTriangleListPtr = std::shared_ptr<IndexedTriangleList>;
public:
	YmTngnDmTriangleMesh();
	virtual ~YmTngnDmTriangleMesh();

	YmTngnDmTriangleMesh(const YmTngnDmTriangleMesh& other) = delete;
	YmTngnDmTriangleMesh& operator = (const YmTngnDmTriangleMesh& other) = delete;

	void AddIndexedTriangleList(const YmTngnIndexedTriangleListPtr& pTriList);

	DirectX::XMFLOAT4X4 GetLocalToGlobalMatrix() const { return *m_pLocalToGlobalMatrix; }
	void SetLocalToGlobalMatrix(const DirectX::XMFLOAT4X4& matrix) { *m_pLocalToGlobalMatrix = matrix; }

	void SetColor(YmRgba4b color);

	std::vector<IndexedTriangleListPtr> FindPickedTriangles(YmTngnPickTargetId id) const;

	YmTngnDmTriangleMeshPtr CreateClone() const;
public:
	static YmTngnDmTriangleMeshPtr MakeSampleData(YmVector3d origin);
	static YmTngnDmTriangleMeshPtr MakeSampleCylinderData(
		double approxTol, YmVector3d origin, YmVector3d axisDir, double radius, double height, YmRgba4b color, bool smooth);

protected:
	virtual bool OnSetPickEnabled(bool isEnable);
	virtual void OnDraw(YmTngnDraw* pDraw);

private:
	YmTngnModelMatrixPtr m_pLocalToGlobalMatrix;
	std::vector<IndexedTriangleListPtr> m_indexedTriangleLists;
};

}
