#pragma once

#include "YmTngnModel.h"
#include "YmBase/YmBinaryFormatter.h"
#include "YmBase/YmWin32FileBuf.h"
#include "YmBase/YmAabBox.h"

namespace Ymcpp {

class YmTngnPointBlockListBuilder
{
public:
	using PointType = YmTngnModel::PointType;
	struct BlockData {
		int64_t blockByteBegin = 0;
		int64_t nPoint = 0;
		YmAabBox3d aabb;
	};
	
public:
	explicit YmTngnPointBlockListBuilder(YmWin32FileBuf* pOutputStreamBuf);
	virtual ~YmTngnPointBlockListBuilder();

	YmTngnPointBlockListBuilder(const YmTngnPointBlockListBuilder& other) = delete;
	YmTngnPointBlockListBuilder(YmTngnPointBlockListBuilder&& other) noexcept = delete;

	YmTngnPointBlockListBuilder& operator = (const YmTngnPointBlockListBuilder& other) = delete;
	YmTngnPointBlockListBuilder& operator = (YmTngnPointBlockListBuilder&& other) noexcept = delete;

public:
	int64_t GetTargetPointCountPerBlock() const { return m_targetPointCountPerBlock; }
	void SetTargetPointCountPerBlock(int64_t nPoint) { YM_IS_TRUE(0 < m_targetPointCountPerBlock);  m_targetPointCountPerBlock = nPoint; }

public:
	void AddPoint(const PointType& point);
	int64_t GetPointCount() const { return m_nInputPoint; }

	void BuildPointBlockFile();

private:
	static void Build1Level(
		YmBinaryFormatter& inputPointFormatter, int64_t nInputPoint, const YmAabBox3d& inputPointAabb,
		int64_t targetVertexCount,
		YmBinaryFormatter& resultFormatter, int64_t resultFileByteBegin, std::vector<BlockData>* pResultBlockList
	);
private:
	YmWin32FileBuf* m_pOutputFile = nullptr;
	YmBinaryFormatter m_output;
	YmWin32FileBuf m_inputPointFile;
	std::unique_ptr<YmBinaryFormatter> m_pInputPointFormatter;
	YmAabBox3d m_inputPointAabb;
	int64_t m_nInputPoint = 0;
private:
	int64_t m_targetPointCountPerBlock = 1024 * 1024;
};

}
