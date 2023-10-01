#include "pch.h"
#include "YmTngnPointBlockListBuilder.h"
#include "YmTngnExclusiveLodPointListCreator.h"
#include "YmBase/YmOrtho3dXform.h"

using namespace std;
using namespace Ymcpp;

////////////////////////////////////////////////////////////////////////////////

YmTngnPointBlockListBuilder::YmTngnPointBlockListBuilder(YmWin32FileBuf* pOutputStreamBuf)
	: m_pOutputFile(pOutputStreamBuf), m_output(pOutputStreamBuf)
{
	YM_IS_TRUE(pOutputStreamBuf != nullptr);
	m_inputPointFile.OpenTempFile();
	m_pInputPointFormatter = make_unique<YmBinaryFormatter>(&m_inputPointFile);
}

YmTngnPointBlockListBuilder::~YmTngnPointBlockListBuilder()
{
}

////////////////////////////////////////////////////////////////////////////////

void YmTngnPointBlockListBuilder::AddPoint(const PointType& point)
{
	YM_ASSERT(sizeof(PointType) == 4 * 4);
	m_pInputPointFormatter->WriteBytes(reinterpret_cast<const char*>(&point), sizeof(PointType));
	m_inputPointAabb.Extend(YmVector3d(point.coord));
	++m_nInputPoint;
}

void YmTngnPointBlockListBuilder::SetScannerPosition(const YmVector3d& pos)
{
	m_pScannerPosition = make_unique<YmVector3d>(pos);
}

////////////////////////////////////////////////////////////////////////////////

/// <summary>
/// Calculate division max so that one lattie would have number of points less than targetVertexCountInLattice.
/// This function doesn't assure that the returned counts would satisfied the condition.
/// </summary>
/// <param name="aabb"></param>
/// <param name="nVertex"></param>
/// <param name="targetVertexCountInLattice"></param>
/// <param name="aResultCount">(output) result division counts</param>
static void CalculateLatticeDivisionCount(
	const YmAabBox3d& aabb, int64_t nVertex, int64_t targetVertexCountInLattice,
	int aResultCount[3]
)
{
	YM_IS_TRUE(aabb.IsInitialized());
	YM_IS_TRUE(0 < targetVertexCountInLattice);
	const int64_t nLatticeRequired = (nVertex + targetVertexCountInLattice - 1) / targetVertexCountInLattice;
	YM_IS_TRUE(nLatticeRequired < (1i64 << 31));

	aResultCount[0] = aResultCount[1] = aResultCount[2] = 1;

	if (nLatticeRequired == 1) {
		return;
	}

	YmVector3d aabbSize = aabb.GetMaxPoint() - aabb.GetMinPoint();

	int minAxis = 0;
	int maxAxis = 0;
	for (int i = 1; i < 3; ++i) {
		if (aabbSize[i] < aabbSize[minAxis]) {
			minAxis = i;
		}
		else if (aabbSize[maxAxis] < aabbSize[i]) {
			maxAxis = i;
		}
	}
	if (minAxis == maxAxis) {
		// all axies are same.
		YM_ASSERT(minAxis == 0);
		maxAxis = (minAxis + 2) % 3;
	}

	const double tolZero = 1e-6;
	if (aabbSize[maxAxis] < tolZero) {
		// cannnot dividied.
		return;
	}

	int maxDivision = 1024;	// TODO: update this value.
	if (nLatticeRequired < maxDivision) {
		maxDivision = (int)nLatticeRequired;
	}
	int aSortedAxis[3] = { minAxis, 3 - minAxis - maxAxis, maxAxis };
	double shorterAxisLengthRatio[2] = { -1, -1 };
	int nZeroLengthAxis = 0;
	for (int i = 0; i < 2; ++i) {
		int axis = aSortedAxis[i];
		shorterAxisLengthRatio[i] = aabbSize[axis] / aabbSize[maxAxis];
		if (aabbSize[axis] * maxDivision < aabbSize[maxAxis]) {
			++nZeroLengthAxis;
		}
	}
	if (nZeroLengthAxis == 2) {
		aResultCount[maxAxis] = static_cast<int>(nLatticeRequired);
		return;
	}

	double cubicRatio = 1;
	for (int i = nZeroLengthAxis; i < 2; ++i) {
		cubicRatio *= shorterAxisLengthRatio[i];
	}
	double latticeLength = aabbSize[maxAxis] / pow(nLatticeRequired / cubicRatio, 1.0 / (3 - nZeroLengthAxis));

	for (int i = nZeroLengthAxis; i < 3; ++i) {
		int axis = aSortedAxis[i];
		aResultCount[axis] = static_cast<int>(ceil(aabbSize[axis] / latticeLength));
	}
}

namespace {
	class Vector3dToLatticeIndex
	{
	public:
		Vector3dToLatticeIndex(const YmAabBox3d& range, const int aDivisionCount[3])
			: m_basePoint(range.GetMinPoint())
		{
			YmVector3d length = range.GetMaxPoint() - m_basePoint;
			for (int i = 0; i < 3; ++i) {
				YM_IS_TRUE(0 < aDivisionCount[i]);
				m_anDivision[i] = aDivisionCount[i];
				m_aBaseLength[i] = length[i] / aDivisionCount[i];
			}
		}

		YmVector3i GetIndices(const YmVector3d& v) const
		{
			YmVector3i ret;
			ret[0] = ToInt(v[0], m_basePoint[0], m_aBaseLength[0], m_anDivision[0]);
			ret[1] = ToInt(v[1], m_basePoint[1], m_aBaseLength[1], m_anDivision[1]);
			ret[2] = ToInt(v[2], m_basePoint[2], m_aBaseLength[2], m_anDivision[2]);
			return ret;
		}

		size_t GetLatticeCount() const
		{
			return size_t(m_anDivision[0]) * m_anDivision[1] * m_anDivision[2];
		}

		size_t GetLatticeIndex(const YmVector3i& indices) const
		{
			size_t ret = indices[2];
			ret *= m_anDivision[1];
			ret += indices[1];
			ret *= m_anDivision[0];
			ret += indices[0];
			return ret;
		}

	private:
		static int ToInt(double v, double v0, double baseLength, int maxValue)
		{
			int i = static_cast<int>((v - v0) / baseLength);
			if (i < 0) {
				return 0;
			}
			else if (maxValue <= i) {
				return maxValue - 1;
			}
			return i;
		}
	private:
		YmVector3d m_basePoint;
		double m_aBaseLength[3] = { 0 };
		int m_anDivision[3] = { 0 };
	};
}

// TODO: This function is ad-hoc. To be fixed.
static double DecideLodBaseLatticeLength(const YmAabBox3d& blockAabb, int64_t numBlockPoint)
{
	int targetPointNumInLattice = 100;
	const int64_t targetLatticeCountUB = 1ui64 << 20;
	if (targetPointNumInLattice * targetLatticeCountUB < numBlockPoint) {
		if (INT_MAX * targetLatticeCountUB < numBlockPoint) {
			YM_THROW_ERROR("Not supported case. numBlockPoint is too large.");
		}
		targetPointNumInLattice = static_cast<int>(numBlockPoint / targetLatticeCountUB);
	}
	int aResultCount[3];
	CalculateLatticeDivisionCount(blockAabb, numBlockPoint, targetPointNumInLattice, aResultCount);

	YmVector3d diagonalVec = blockAabb.GetMaxPoint() - blockAabb.GetMinPoint();
	const double baseLengthMin = 0.01;
	double baseLength = HUGE_VAL;
	for (int i = 0; i < 3; ++i) {
		YM_IS_TRUE(0 < aResultCount[i]);
		double len = diagonalVec[i] / aResultCount[i];
		if (len < baseLengthMin) {
			len = baseLengthMin;
		}
		if (len < baseLength) {
			baseLength = len;
		}
	}
	return baseLength;
}

void YmTngnPointBlockListBuilder::BuildPointBlockFile()
{
	YM_IS_TRUE(m_pInputPointFormatter);
	m_pInputPointFormatter->Flush();

	// TODO: maintain output format flag.
	//m_output.SetFormatFlags(0);

	int64_t targetPointCount = GetTargetPointCountPerBlock();

	YmWin32FileBuf dividedFile;
	dividedFile.OpenTempFile();
	auto dividedFileFormatter = YmBinaryFormatter::CreateForMemoryImage(&dividedFile);
	vector<BlockData> dividedBlocks;
	Build1Level(*m_pInputPointFormatter, GetPointCount(), m_inputPointAabb, targetPointCount,
		dividedFileFormatter, 0, &dividedBlocks);

	size_t nBlock = dividedBlocks.size();
	YM_IS_TRUE(nBlock < (size_t(1) << 31));

	YmTngnModel::FileHeader fileHeader = {
		YmTngnModel::CURRENT_FILE_VERSION, m_output.GetFormatFlags(), "PointBlockList", 0
	};
	// Write temporary data. To be over written, later.
	int64_t headerBeginPos = m_output.GetCurrentPosition();
	fileHeader.WriteTo(m_output.GetStreamBuf());
	const int64_t headerEndPos = m_output.GetCurrentPosition();

	int64_t endOfFilePos = headerEndPos;

	using PointBlockHeader = YmTngnModel::PointBlockHeader;
	vector<YmTngnModel::PointBlockHeader> blockImages;
	blockImages.resize(nBlock);
	for (size_t iBlock = 0; iBlock < nBlock; ++iBlock) {
		const BlockData& inBlock = dividedBlocks[iBlock];
		PointBlockHeader& image = blockImages[iBlock];

		YmOrtho3dXform<double>().GetRowMajorMatrix(image.localToGlobalMatrix);
		YmVectorUtil::CopyToArray(inBlock.aabb.GetMinPoint(), 3, image.aAabbPoint[0]);
		YmVectorUtil::CopyToArray(inBlock.aabb.GetMaxPoint(), 3, image.aAabbPoint[1]);
		image.firstBytePos = endOfFilePos;

		m_output.SetCurrentPosition(endOfFilePos);
		const double latticeLength = DecideLodBaseLatticeLength(inBlock.aabb, inBlock.nPoint);
		YmTngnExclusiveLodPointListCreator creator(latticeLength);
		image.nImageByte = creator.CreateImage(
			dividedFileFormatter, inBlock.blockByteBegin, inBlock.nPoint,
			inBlock.aabb, m_pOutputFile
		);
		endOfFilePos += image.nImageByte;
	}

	m_output.SetCurrentPosition(endOfFilePos);

	// write list contents
	int64_t blockHeaderListBeginPos = endOfFilePos;
	m_output.WriteInt32(static_cast<int32_t>(nBlock));
	for (size_t iBlock = 0; iBlock < nBlock; ++iBlock) {
		PointBlockHeader& image = blockImages[iBlock];
		m_output.WriteBytes(reinterpret_cast<const char*>(&image), sizeof(PointBlockHeader));
	}
	if (m_pScannerPosition) {
		m_output.WriteInt8(1);
		YmTngnModel::WriteVector3d(m_output, *m_pScannerPosition);
	}
	else {
		m_output.WriteInt8(0);
		YmTngnModel::WriteVector3d(m_output, YmVector3d::MakeZero());
	}
	int64_t listCntentsEndPos = m_output.GetCurrentPosition();

	// update file header.
	fileHeader.contentPosition = blockHeaderListBeginPos;
	m_output.SetCurrentPosition(headerBeginPos);
	fileHeader.WriteTo(m_output.GetStreamBuf());
	YM_IS_TRUE(m_output.GetCurrentPosition() == headerEndPos);

	m_output.SetCurrentPosition(listCntentsEndPos);	// back to the end of the output image.
}

////////////////////////////////////////////////////////////////////////////////

static void st_WritePonitBuffer(
	YmTngnPointBlockListBuilder::BlockData& currBlock, vector<YmTngnPointBlockListBuilder::PointType>& pointWriteBuffer,
	YmBinaryFormatter& resultFormatter
)
{
	if (pointWriteBuffer.empty()) {
		return;
	}
	using PointType = YmTngnPointBlockListBuilder::PointType;
	resultFormatter.SetCurrentPosition(currBlock.blockByteBegin + currBlock.nPoint * sizeof(PointType));
	resultFormatter.WriteBytes(
		reinterpret_cast<const char*>(pointWriteBuffer.data()), pointWriteBuffer.size() * sizeof(PointType)
	);
	currBlock.nPoint += pointWriteBuffer.size();
	pointWriteBuffer.clear();
}

void YmTngnPointBlockListBuilder::Build1Level(
	YmBinaryFormatter& inputPointFormatter, int64_t nInputPoint, const YmAabBox3d& inputPointAabb,
	int64_t targetVertexCount,
	YmBinaryFormatter& resultFormatter, int64_t resultFileByteBegin, std::vector<BlockData>* pResultBlockList
)
{
	int aDivisionCount[3] = { 0 };
	CalculateLatticeDivisionCount(inputPointAabb, nInputPoint, targetVertexCount, aDivisionCount);

	Vector3dToLatticeIndex toLatticeIndex(inputPointAabb, aDivisionCount);
	size_t nLattice = toLatticeIndex.GetLatticeCount();

	vector<int64_t> latticePointCounts;
	latticePointCounts.resize(nLattice, 0);

	inputPointFormatter.SetCurrentPosition(0);
	for (int64_t iPoint = 0; iPoint < nInputPoint; ++iPoint) {
		PointType point;
		inputPointFormatter.ReadBytes(reinterpret_cast<char*>(&point), sizeof(PointType));

		YmVector3i latticeIndices = toLatticeIndex.GetIndices(point.coord);
		size_t latticeIndex = toLatticeIndex.GetLatticeIndex(latticeIndices);
		latticePointCounts[latticeIndex]++;
	}

	vector<BlockData> resultBlockList;
	vector<vector<PointType>> pointWriteBufferList;
	resultBlockList.resize(nLattice);
	pointWriteBufferList.resize(nLattice);
	int64_t nextPointBegin = 0;
	for (size_t iLattice = 0; iLattice < nLattice; ++iLattice) {
		resultBlockList[iLattice].blockByteBegin = resultFileByteBegin + nextPointBegin * sizeof(PointType);
		pointWriteBufferList[iLattice].reserve(100);
		nextPointBegin += latticePointCounts[iLattice];
	}

	inputPointFormatter.SetCurrentPosition(0);
	for (int64_t iVertex = 0; iVertex < nInputPoint; ++iVertex) {
		PointType point;
		inputPointFormatter.ReadBytes(reinterpret_cast<char*>(&point), sizeof(PointType));

		YmVector3d pointPos = point.coord;
		YmVector3i latticeIndices = toLatticeIndex.GetIndices(pointPos);
		size_t latticeIndex = toLatticeIndex.GetLatticeIndex(latticeIndices);

		BlockData& currBlock = resultBlockList[latticeIndex];
		YM_IS_TRUE(currBlock.nPoint < latticePointCounts[latticeIndex]);
		auto& pointWriteBuffer = pointWriteBufferList[latticeIndex];

		if (pointWriteBuffer.size() == pointWriteBuffer.capacity()) {
			st_WritePonitBuffer(currBlock, pointWriteBuffer, resultFormatter);
		}

		YM_ASSERT(pointWriteBuffer.size() < pointWriteBuffer.capacity());
		pointWriteBuffer.push_back(point);
		currBlock.aabb.Extend(pointPos);
	}

	for (size_t iLattice = 0; iLattice < nLattice; ++iLattice) {
		st_WritePonitBuffer(resultBlockList[iLattice], pointWriteBufferList[iLattice], resultFormatter);
	}

	resultBlockList.erase(
		remove_if(
			resultBlockList.begin(), resultBlockList.end(),
			[](const BlockData& block) -> bool { return block.nPoint == 0; }
		),
		resultBlockList.end()
	);

	*pResultBlockList = move(resultBlockList);
}

////////////////////////////////////////////////////////////////////////////////
