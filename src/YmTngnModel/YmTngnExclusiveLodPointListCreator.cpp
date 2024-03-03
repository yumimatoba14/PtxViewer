#include "pch.h"
#include "YmTngnExclusiveLodPointListCreator.h"
#include "YmTngnExclusiveLodPointListLodTable.h"

using namespace std;
using namespace Ymcpp;

const double MIN_BASE_LENGTH = 1e-6;

////////////////////////////////////////////////////////////////////////////////

YmTngnExclusiveLodPointListCreator::~YmTngnExclusiveLodPointListCreator()
{
}

////////////////////////////////////////////////////////////////////////////////

namespace {
	class VectorToLatticeIndex
	{
	public:
		VectorToLatticeIndex(const YmAabBox3d& range, double baseLength)
			: m_basePoint(range.GetMinPoint()), m_baseLength(baseLength)
		{
			YmVector3d length = range.GetMaxPoint() - m_basePoint;
			for (int i = 0; i < 3; ++i) {
				m_anDivision[i] = static_cast<int>(ceil(length[i] / m_baseLength));
				if (m_anDivision[i] < 1) {
					m_anDivision[i] = 1;
				}
			}
		}

		YmVector3d GetLatticeCenter(const YmVector3i& indices) const
		{
			double dHalfLength = 0.5 * m_baseLength;
			double aCoord[3] = {
				m_basePoint[0] + indices[0] * m_baseLength + dHalfLength,
				m_basePoint[1] + indices[1] * m_baseLength + dHalfLength,
				m_basePoint[2] + indices[2] * m_baseLength + dHalfLength
			};
			return YmVector3d(aCoord);
		}

		YmVector3i GetIndices(const YmVector3d& v) const
		{
			YmVector3i ret;
			ret[0] = ToInt(v[0], m_basePoint[0], m_anDivision[0]);
			ret[1] = ToInt(v[1], m_basePoint[1], m_anDivision[1]);
			ret[2] = ToInt(v[2], m_basePoint[2], m_anDivision[2]);
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
		int ToInt(double v, double v0, int n) const
		{
			int i = static_cast<int>((v - v0) / m_baseLength);
			if (i < 0) {
				return 0;
			}
			else if (n <= i) {
				return n - 1;
			}
			return i;
		}
	private:
		YmVector3d m_basePoint;
		double m_baseLength;
		int m_anDivision[3];
	};

	class LatticeInfo
	{
	public:
		bool hasPoint = false;
		double sqDistanceFromCenter;
	};

	class RemainingPointStorage
	{
	public:
		typedef YmTngnExclusiveLodPointListCreator::PointType Point;

		RemainingPointStorage(YmBinaryFormatter* pFile)
			: m_nWrittenPoint(0), m_pFile(pFile)
		{
			YM_ASSERT(m_pFile);
			m_pointBuffer.reserve(1024 * 4);
			m_outputByteBegin = pFile->GetCurrentPosition();
		}

		~RemainingPointStorage()
		{
			Flush();
		}

		void AddPoint(const Point& vertex)
		{
			if (m_pointBuffer.size() == m_pointBuffer.capacity()) {
				Flush();
			}
			YM_ASSERT(m_pointBuffer.size() < m_pointBuffer.capacity());
			m_pointBuffer.push_back(vertex);
		}

		void Flush()
		{
			if (m_pointBuffer.empty()) {
				return;
			}
			int64_t orgPos = m_pFile->GetCurrentPosition();
			YM_IS_TRUE(int64_t((m_nWrittenPoint + m_pointBuffer.size()) * sizeof(Point)) < orgPos);

			m_pFile->SetCurrentPosition(m_outputByteBegin + m_nWrittenPoint * sizeof(Point));
			DWORD nWrittenByte = 0;
			m_pFile->WriteBytes(reinterpret_cast<const char*>(m_pointBuffer.data()), m_pointBuffer.size() * sizeof(Point));
			m_nWrittenPoint += m_pointBuffer.size();
			m_pointBuffer.clear();

			m_pFile->SetCurrentPosition(orgPos);
		}

		int64_t GetPointCount() const { return m_nWrittenPoint + m_pointBuffer.size(); }

	private:
		int64_t m_outputByteBegin;
		int64_t m_nWrittenPoint;
		vector<Point> m_pointBuffer;
		YmBinaryFormatter* m_pFile;
	};
}

int64_t YmTngnExclusiveLodPointListCreator::CreateImage(
	YmBinaryFormatter& inputPointFormatter, int64_t inputPointFileByteBegin, int64_t nInputPoint,
	const YmAabBox3d& inputPointAabb,
	YmWin32FileBuf* pResultFileBuf
)
{
	YM_IS_TRUE(MIN_BASE_LENGTH < m_latticeLength);
	YM_IS_TRUE(pResultFileBuf != nullptr && pResultFileBuf->IsOpened());

	using HeaderType = YmTngnModel::ExclusiveLodPointList::Header;
	HeaderType header = { YmTngnModel::CURRENT_FILE_VERSION, 0 };

	YmBinaryFormatter resultFileFormatter = YmBinaryFormatter::CreateForMemoryImage(pResultFileBuf);
	int64_t heaerPos = resultFileFormatter.GetCurrentPosition();

	// Write temporary value.
	resultFileFormatter.WriteBytes(reinterpret_cast<const char*>(&header), sizeof(HeaderType));

	YmTngnExclusiveLodPointListLodTable lodTable;
	const int nLevelOfLattice = 4;
	{
		const double aLength[] = {
			m_latticeLength, m_latticeLength * 0.5, m_latticeLength * 0.25, m_latticeLength * 0.125
		};
		for (int i = 0; i < nLevelOfLattice; ++i) {
			lodTable.SetLevelInfo(aLength[i], nInputPoint);
		}
	}
	lodTable.SetLevelInfo(MIN_BASE_LENGTH, nInputPoint);	// centinel to draw all vertices.
	int64_t pointListBeginPos = resultFileFormatter.GetCurrentPosition();
	int64_t pointListEndPos = pointListBeginPos + nInputPoint * sizeof(PointType);

	YmMemoryMappedFile resultMmFile;
	resultMmFile.AttachFileToWrite(pResultFileBuf->GetHandle(), pointListEndPos);

	int64_t nCurrVertex = nInputPoint;
	int64_t levelDataBegin = pointListBeginPos;
	double latticeLength = lodTable.GetFirstLevelLength();
	for (int i = 0; i < nLevelOfLattice; ++i) {
		inputPointFormatter.SetCurrentPosition(inputPointFileByteBegin);
		int64_t nAddedLattice = Build1Level(
			inputPointFormatter, nCurrVertex, inputPointAabb, latticeLength, resultMmFile, levelDataBegin
		);
		if (nAddedLattice == 0) {
			// vertices are less than making lattices.
			break;
		}
		YM_IS_TRUE(nAddedLattice <= nCurrVertex);
		nCurrVertex -= nAddedLattice;
		lodTable.SetLevelInfo(latticeLength, nInputPoint - nCurrVertex);

		levelDataBegin += nAddedLattice * sizeof(PointType);
		latticeLength = lodTable.GetNextLevelLength(latticeLength);
	}

	if (0 < nCurrVertex) {
		inputPointFormatter.SetCurrentPosition(inputPointFileByteBegin);
		CopyVertices(inputPointFormatter, nCurrVertex, resultMmFile, levelDataBegin, pointListEndPos);
	}

	resultMmFile.Close();
	resultFileFormatter.SetCurrentPosition(pointListEndPos);
	lodTable.WriteTo(resultFileFormatter);

	int64_t resultFileEndPos = resultFileFormatter.GetCurrentPosition();
	header.lodTablePos = pointListEndPos;
	resultFileFormatter.SetCurrentPosition(heaerPos);
	resultFileFormatter.WriteBytes(reinterpret_cast<const char*>(&header), sizeof(HeaderType));
	YM_IS_TRUE(resultFileFormatter.GetCurrentPosition() == pointListBeginPos);

	return (int64_t)(resultFileEndPos - heaerPos);
}

////////////////////////////////////////////////////////////////////////////////

int64_t YmTngnExclusiveLodPointListCreator::Build1Level(
	YmBinaryFormatter& inputPointFormatter, int64_t nInputPoint,
	const YmAabBox3d& inputPointAabb, double latticeLength,
	YmMemoryMappedFile& resultFile, int64_t resultFileBeginPos
)
{
	VectorToLatticeIndex toLatticeIndex(inputPointAabb, latticeLength);
	const size_t nLattice = toLatticeIndex.GetLatticeCount();
	if (nInputPoint < (int64_t)nLattice) {
		// There is not enough space to allocate points of lattices.
		return 0;
	}

	// Assign vertices to the lattice.
	// Not assigned vertices are stored to nextLevelPoints.
	RemainingPointStorage nextLavelPoints(&inputPointFormatter);

	YmMemoryMappedFile::MappedPtr pLatticePointMemory = resultFile.MapView(resultFileBeginPos, nLattice * sizeof(PointType));
	PointType* aLatticePoint = pLatticePointMemory.ToArray<PointType>();
	vector<LatticeInfo> latticeInfos;
	latticeInfos.resize(nLattice);

	for (int64_t iVertex = 0; iVertex < nInputPoint; ++iVertex) {
		PointType vertex;
		inputPointFormatter.ReadBytes(reinterpret_cast<char*>(&vertex), sizeof(PointType));

		YmVector3d vertexPos = vertex.coord;
		YmVector3i vtxIndices = toLatticeIndex.GetIndices(vertexPos);
		size_t latticeIndex = toLatticeIndex.GetLatticeIndex(vtxIndices);
		double sqDistanceFromCenter = (vertexPos - toLatticeIndex.GetLatticeCenter(vtxIndices)).GetSqLength();

		auto& info = latticeInfos[latticeIndex];
		bool setToLattice = true;		// true to set vertex to the lattice.
		bool addToNextLevel = false;	// true when either of 2 vertices are used in the next level.
		if (info.hasPoint) {
			// Choose the vertex of which sqDistanceFromCenter is the minimum.
			setToLattice = (sqDistanceFromCenter < info.sqDistanceFromCenter);
			addToNextLevel = true;
		}
		if (setToLattice) {
			info.hasPoint = true;
			info.sqDistanceFromCenter = sqDistanceFromCenter;
			std::swap(aLatticePoint[latticeIndex], vertex);
		}

		if (addToNextLevel) {
			nextLavelPoints.AddPoint(vertex);
		}
	}

	bool isNeedStrip = nInputPoint < int64_t(nLattice) + nextLavelPoints.GetPointCount();

	size_t nAddedLattice = nLattice;
	if (isNeedStrip) {
		// There is more than 1 lattice which has no point.
		// Strip such lattices and compress memory area so that lattices in the former area
		// would have vertices and ones in the latter wouldn't.
		size_t emptyLattice = 0;
		size_t filledLatticeEnd = nLattice;
		while (emptyLattice < filledLatticeEnd) {
			while (emptyLattice < filledLatticeEnd && latticeInfos[emptyLattice].hasPoint) {
				++emptyLattice;
			}
			while (emptyLattice < filledLatticeEnd && !latticeInfos[filledLatticeEnd - 1].hasPoint) {
				YM_ASSERT(0 < filledLatticeEnd);
				--filledLatticeEnd;
			}
			YM_ASSERT(emptyLattice <= filledLatticeEnd);

			if (emptyLattice < filledLatticeEnd) {
				YM_ASSERT(emptyLattice + 1 < filledLatticeEnd);
				swap(aLatticePoint[emptyLattice], aLatticePoint[filledLatticeEnd - 1]);
				++emptyLattice;
				--filledLatticeEnd;
				YM_ASSERT(emptyLattice <= filledLatticeEnd);
			}
			else {
				break;
			}
		}
		YM_ASSERT(emptyLattice == filledLatticeEnd);

		nAddedLattice = emptyLattice;
	}

	return nAddedLattice;
}

void YmTngnExclusiveLodPointListCreator::CopyVertices(
	YmBinaryFormatter& inputPointFormatter, int64_t nInputPoint,
	YmMemoryMappedFile& resultFile, int64_t resultFileBeginPos, int64_t resultFileEndPos
)
{
	YM_ASSERT(int64_t(resultFileBeginPos + nInputPoint * sizeof(PointType)) <= resultFileEndPos);
	const size_t nPointInLatterBuf = 1024 * 4;
	const size_t latterBufSize = nPointInLatterBuf * sizeof(PointType);
	YmMemoryMappedFile::MappedPtr pLatterMemory;

	for (int64_t iVertex = 0; iVertex < nInputPoint; ++iVertex) {
		PointType vertex;
		inputPointFormatter.ReadBytes(reinterpret_cast<char*>(&vertex), sizeof(PointType));

		size_t indexInBuf = iVertex % nPointInLatterBuf;
		if (indexInBuf == 0) {
			int64_t bufferBegin = resultFileBeginPos + iVertex * sizeof(PointType);
			size_t bufSize = latterBufSize;
			if (resultFileEndPos < bufferBegin + int64_t(bufSize)) {
				bufSize = static_cast<size_t>(resultFileEndPos - bufferBegin);
			}
			pLatterMemory = resultFile.MapView(bufferBegin, bufSize);
			YM_IS_TRUE(!pLatterMemory.IsNull());
		}
		pLatterMemory.ToArray<PointType>()[indexInBuf] = vertex;
	}
}

////////////////////////////////////////////////////////////////////////////////
