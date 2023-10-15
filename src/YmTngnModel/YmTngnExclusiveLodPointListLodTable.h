#pragma once

#include "YmTngnModel.h"
#include "YmBase/YmMemoryMappedFile.h"
#include <map>

namespace Ymcpp {

class YmTngnExclusiveLodPointListLodTable
{
private:
	struct LevelInfo {
		double baseLength;
		int64_t nPoint;

		LevelInfo() = default;
		LevelInfo(double len, int64_t n) : baseLength(len), nPoint(n) {}
	};

	using LodTable = std::map<double, LevelInfo, std::greater<double> >;

public:
	YmTngnExclusiveLodPointListLodTable();
	virtual ~YmTngnExclusiveLodPointListLodTable();

	size_t GetLevelCount() const { return m_levelInfoTable.size(); }
	void ClearLevelInfo() { m_levelInfoTable.clear(); }
	void SetLevelInfo(double baseLength, int64_t nPoint);
	int64_t GetPointCount() const;
	int64_t GetEnoughPointCount(double baseLength) const;
	double GetFirstLevelLength() const;
	double GetNextLevelLength(double currLength) const;

public:
	void WriteTo(YmBinaryFormatter& output) const;
	void ReadFrom(YmBinaryFormatter& input);
	void ReadFrom(YmMemoryMappedFile& inputFile, int64_t pos);
private:
	LodTable m_levelInfoTable;
};

}
