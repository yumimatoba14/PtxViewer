#include "pch.h"
#include "YmTngnExclusiveLodPointListLodTable.h"
#include "YmBase/YmMemoryBinaryFormatter.h"

using namespace std;
using namespace Ymcpp;

static constexpr size_t MAX_LEVEL_COUNT = 255;
static constexpr size_t LEVEL_INFO_SIZE = 8 + 8;

////////////////////////////////////////////////////////////////////////////////

YmTngnExclusiveLodPointListLodTable::YmTngnExclusiveLodPointListLodTable()
{
}

YmTngnExclusiveLodPointListLodTable::~YmTngnExclusiveLodPointListLodTable()
{
}

////////////////////////////////////////////////////////////////////////////////

void YmTngnExclusiveLodPointListLodTable::SetLevelInfo(double baseLength, int64_t nPoint)
{
	YM_ASSERT(m_levelInfoTable.size() < MAX_LEVEL_COUNT);
	m_levelInfoTable[baseLength] = { baseLength, nPoint };
}

int64_t YmTngnExclusiveLodPointListLodTable::GetPointCount() const
{
	if (m_levelInfoTable.empty()) {
		return 0;
	}
	return m_levelInfoTable.rbegin()->second.nPoint;
}

int64_t YmTngnExclusiveLodPointListLodTable::GetEnoughPointCount(double baseLength) const
{
	auto itLb = m_levelInfoTable.lower_bound(baseLength);
	if (itLb == m_levelInfoTable.end()) {
		return GetPointCount();
	}
	return itLb->second.nPoint;
}

double YmTngnExclusiveLodPointListLodTable::GetFirstLevelLength() const
{
	YM_IS_TRUE(!m_levelInfoTable.empty());
	return m_levelInfoTable.begin()->first;
}

/// <summary>
/// Get the next level.
/// </summary>
/// <param name="currLength"></param>
/// <returns>
/// Base length of the next smaller level.
/// Return currLength if it is the last level.
/// </returns>
double YmTngnExclusiveLodPointListLodTable::GetNextLevelLength(double currLength) const
{
	YM_IS_TRUE(!m_levelInfoTable.empty());
	auto itLb = m_levelInfoTable.lower_bound(currLength);
	if (itLb == m_levelInfoTable.end()) {
		return m_levelInfoTable.rbegin()->first;
	}
	// NOTE: m_levelInfos is ordered in descending order.
	if (currLength > itLb->first) {
		return itLb->first;
	}
	++itLb;
	if (itLb == m_levelInfoTable.end()) {
		return m_levelInfoTable.rbegin()->first;
	}
	return itLb->first;
}

////////////////////////////////////////////////////////////////////////////////

void YmTngnExclusiveLodPointListLodTable::WriteTo(YmBinaryFormatter& output) const
{
	YM_IS_TRUE(m_levelInfoTable.size() <= MAX_LEVEL_COUNT);
	uint8_t nLevel = static_cast<uint8_t>(m_levelInfoTable.size());

	output.WriteInt8(nLevel);

	// output in descending order of baseLength.
	for (auto it = m_levelInfoTable.begin(); it != m_levelInfoTable.end(); ++it) {
		const LevelInfo& level = it->second;
		output.WriteDouble(level.baseLength);
		output.WriteInt64(level.nPoint);
	}
}

void YmTngnExclusiveLodPointListLodTable::ReadFrom(YmBinaryFormatter& input)
{
	size_t nLevel = static_cast<uint8_t>(input.ReadInt8());
	m_levelInfoTable.clear();
	for (size_t iLevel = 0; iLevel < nLevel; ++iLevel) {
		double baseLength = input.ReadDouble();
		int64_t nPoint = input.ReadInt64();
		SetLevelInfo(baseLength, nPoint);
	}
}

void YmTngnExclusiveLodPointListLodTable::ReadFrom(YmMemoryMappedFile& inputFile, int64_t pos)
{
	size_t dataSize = 32;
	auto pSrc = inputFile.MapView(pos, dataSize);

	YmMemoryBinaryFormatter buffer;
	buffer.WriteBytes(pSrc.ToConstArray<char>(), dataSize);
	buffer.SetCurrentPosition(0);
	uint8_t nLevel = static_cast<uint8_t>(buffer.ReadInt8());

	size_t dataSizeActual = 1 + nLevel * LEVEL_INFO_SIZE;
	if (dataSize < dataSizeActual) {
		pSrc = inputFile.MapView(pos, dataSizeActual);
		buffer.SetCurrentPosition(0);
		buffer.WriteBytes(pSrc.ToConstArray<char>(), dataSizeActual);
	}
	buffer.SetCurrentPosition(0);
	ReadFrom(buffer);
}

////////////////////////////////////////////////////////////////////////////////
