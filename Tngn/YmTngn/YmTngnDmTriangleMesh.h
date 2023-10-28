#pragma once

#include "YmTngn/YmTngnDrawingModel.h"

namespace Ymcpp {

class YmTngnDmTriangleMesh : public YmTngnDrawingModel
{
public:
	using VertexType = YmTngnTriangleVertex;
	using IndexType = UINT;

	class IndexedTriangleList
	{
	public:
		IndexedTriangleList(YmTngnIndexedTriangleListPtr pModel) : m_pModel(std::move(pModel)) {}

		void Draw(YmTngnDraw* pDraw);
	private:
		void PrepareData(YmTngnDraw* pDraw);
	private:
		YmTngnIndexedTriangleListPtr m_pModel;
		D3DBufferPtr m_pVertexBuffer;
		D3DBufferPtr m_pIndexBuffer;
		size_t m_nIndex = 0;
	};
	using IndexedTriangleListPtr = std::shared_ptr<IndexedTriangleList>;
public:
	YmTngnDmTriangleMesh();
	virtual ~YmTngnDmTriangleMesh();

	YmTngnDmTriangleMesh(const YmTngnDmTriangleMesh& other) = delete;
	YmTngnDmTriangleMesh& operator = (const YmTngnDmTriangleMesh& other) = delete;

	void AddIndexedTriangleList(const YmTngnIndexedTriangleListPtr& pTriList);

	DirectX::XMFLOAT4X4 GetLocalToGlobalMatrix() const { return m_localToGlobalMatrix; }
	void SetLocalToGlobalMatrix(const DirectX::XMFLOAT4X4& matrix) { m_localToGlobalMatrix = matrix; }

	static std::shared_ptr<YmTngnDmTriangleMesh> MakeSampleData(YmVector3d origin);

protected:
	//virtual bool OnSetPickEnabled(bool bEnable);
	virtual void OnDraw(YmTngnDraw* pDraw);
	//virtual std::vector<YmTngnPointListVertex> OnFindPickedPoints(YmTngnPickTargetId id);

private:
	DirectX::XMFLOAT4X4 m_localToGlobalMatrix;	// row-major, pre-multiplied
	std::vector<IndexedTriangleListPtr> m_indexedTriangleLists;
};

}
