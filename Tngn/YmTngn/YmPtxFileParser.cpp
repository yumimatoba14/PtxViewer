#include "pch.h"
#include "YmPtxFileParser.h"

using namespace std;
using namespace Ymcpp;

////////////////////////////////////////////////////////////////////////////////

YmPtxFileParser::YmPtxFileParser()
{
	m_aRadiusRange[0] = -1;
	m_aRadiusRange[1] = DBL_MAX;
}

YmPtxFileParser::~YmPtxFileParser()
{
}

////////////////////////////////////////////////////////////////////////////////

YmPtxFileParser::FileHeader YmPtxFileParser::ParseFile(const char* pPtxFilePath, const ParsePointCallback& onParsePoint)
{
	YM_IS_TRUE(pPtxFilePath != nullptr);
	YmUniqueFilePtr pInFile(fopen(pPtxFilePath, "r"));
	if (!pInFile) {
		YM_THROW_ERROR("Failed to open ptx file.");
	}

	FileHeader header;

	const int nBufferByte = 1024;
	char aBufferByte[nBufferByte];
	auto readLine = [&]() {
		char* ret = fgets(aBufferByte, nBufferByte, pInFile.get());
		if (ret == nullptr || static_cast<size_t>(nBufferByte - 1) <= strlen(aBufferByte)) {
			YM_THROW_ERROR("Failed to read line.");
		}
	};
	auto readSize = [&]() {
		readLine();
		int64_t value;
		int ret = sscanf(aBufferByte, "%I64d", &value);
		YM_IS_TRUE(ret == 1);
		return (size_t)value;
	};
	auto readVector3d = [&]() {
		readLine();
		double aValue[3];
		int ret = sscanf(aBufferByte, "%lf %lf %lf", aValue + 0, aValue + 1, aValue + 2);
		YM_IS_TRUE(ret == 3);
		return YmVectorUtil::Make(aValue[0], aValue[1], aValue[2]);
	};
	auto readDouble4 = [&](double aValue[4]) {
		readLine();
		int ret = sscanf(aBufferByte, "%lf %lf %lf %lf", aValue + 0, aValue + 1, aValue + 2, aValue + 3);
		YM_IS_TRUE(ret == 4);
	};

	header.numberOfColumn = readSize();
	header.numberOfRow = readSize();
	header.scannerOrg = readVector3d();
	header.scannerDirX = readVector3d();
	header.scannerDirY = readVector3d();
	header.scannerDirZ = readVector3d();
	readDouble4(header.transform + 0);
	readDouble4(header.transform + 4);
	readDouble4(header.transform + 8);
	readDouble4(header.transform + 12);

	if (onParsePoint) {
		auto readPointData = [&]() {
			readLine();
			PointData point;
			double aValue[3];
			int ret = sscanf(aBufferByte, "%lf %lf %lf %lf %d %d %d",
				aValue + 0, aValue + 1, aValue + 2, &(point.intensity), point.rgb + 0, point.rgb + 1, point.rgb + 2
			);
			YM_IS_TRUE(ret == 7);
			point.localPoint = YmVectorUtil::Make(aValue[0], aValue[1], aValue[2]);
			return point;
		};

		double aRadiusSq[2] = {
			m_aRadiusRange[0] * m_aRadiusRange[0], m_aRadiusRange[1] * m_aRadiusRange[1]
		};
		if (m_aRadiusRange[0] < 0) {
			aRadiusSq[0] *= -1;
		}
		for (size_t iCol = 0; iCol < header.numberOfColumn; ++iCol) {
			for (size_t iRow = 0; iRow < header.numberOfRow; ++iRow) {
				PointData point = readPointData();
				double sqLen = point.localPoint.GetSqLength();
				if (sqLen < aRadiusSq[0] || aRadiusSq[1] < sqLen) {
					continue;
				}

				onParsePoint(header, iCol, iRow, point);
			}
		}
	}
	return header;
}

////////////////////////////////////////////////////////////////////////////////
