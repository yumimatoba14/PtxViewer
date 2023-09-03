#pragma once

#include "YmTngnModel.h"
#include "YmBase/YmBinaryFormatter.h"
#include "YmBase/YmVector.h"
#include "YmBase/YmAabBox.h"
#include "YmBase/YmMemoryMappedFile.h"
#include "YmBase/YmWin32FileBuf.h"

namespace Ymcpp {

class YmTngnExclusiveLodPointListCreator
{
public:
	using PointType = YmTngnModel::PointType;
public:
	explicit YmTngnExclusiveLodPointListCreator(double latticeLength)
		: m_latticeLength(latticeLength)
	{}
	virtual ~YmTngnExclusiveLodPointListCreator();

	YmTngnExclusiveLodPointListCreator(const YmTngnExclusiveLodPointListCreator& other) = delete;
	YmTngnExclusiveLodPointListCreator(YmTngnExclusiveLodPointListCreator&& other) noexcept = delete;

	YmTngnExclusiveLodPointListCreator& operator = (const YmTngnExclusiveLodPointListCreator& other) = delete;
	YmTngnExclusiveLodPointListCreator& operator = (YmTngnExclusiveLodPointListCreator&& other) noexcept = delete;

public:
	int64_t CreateImage(
		YmBinaryFormatter& inputPointFormatter, int64_t inputPointFileByteBegin, int64_t nInputPoint,
		const YmAabBox3d& inputPointAabb,
		YmWin32FileBuf* pResultFileBuf
	);

private:
	static int64_t Build1Level(
		YmBinaryFormatter& inputPointFormatter, int64_t nInputPoint,
		const YmAabBox3d& inputPointAabb, double latticeLength,
		YmMemoryMappedFile& resultFile, int64_t resultFileBeginPos
	);
	static void CopyVertices(
		YmBinaryFormatter& inputPointFormatter, int64_t nInputPoint,
		YmMemoryMappedFile& resultFile, int64_t resultFileBeginPos, int64_t resultFileEndPos
	);

private:
	double m_latticeLength;
};

}
