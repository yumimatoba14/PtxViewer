#pragma once

#include "YmBase/YmVector.h"
#include <functional>

namespace Ymcpp {

class YmPtxFileParser
{
public:
	struct FileHeader
	{
		size_t numberOfColumn;
		size_t numberOfRow;
		YmVector3d scannerOrg;
		YmVector3d scannerDirX;
		YmVector3d scannerDirY;
		YmVector3d scannerDirZ;
		double transform[16];	// row major, pre-multiplied
	};

	struct PointData {
		YmVector3d localPoint;
		double intensity;
		int rgb[3];
	};

	using ParsePointCallback = std::function<
		void(const FileHeader& header, size_t col, size_t row, const PointData& point)>;
public:
	YmPtxFileParser();
	virtual ~YmPtxFileParser();

	YmPtxFileParser(const YmPtxFileParser& other) = delete;
	YmPtxFileParser(YmPtxFileParser&& other) noexcept = delete;

	YmPtxFileParser& operator = (const YmPtxFileParser& other) = delete;
	YmPtxFileParser& operator = (YmPtxFileParser&& other) noexcept = delete;

public:
	double GetRadiusLowerBound() const { return m_aRadiusRange[0]; }
	void SetRadiusLowerBound(double minDist) { m_aRadiusRange[0] = minDist; }
	double GetRadiusUpperBound() const { return m_aRadiusRange[1]; }
	void SetRadiusUpperBound(double maxDist) { m_aRadiusRange[1] = maxDist; }
	FileHeader ParseFile(const char* ptxFilePath, const ParsePointCallback& onParsePoint);
private:
	double m_aRadiusRange[2];
};

}
