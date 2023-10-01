#pragma once

#include "YmBase/YmBaseFwd.h"
#include "YmBase/YmVector.h"
#include "YmTngnModel/YmRgba.h"
#include <map>

namespace Ymcpp {

class YmTngnModel
{
public:
	enum class HeaderCode : int32_t {
		FILE_HEADER = 0x01,
	};

	// CURRENT_FILE_VERSION usually incremnts by 10.
	static constexpr int32_t CURRENT_FILE_VERSION = 10;

#if defined(_M_IX86) || defined(_M_X64)
	static constexpr char* HEAD_TEXT_POINT_BLOCK_LIST = "PointBlockList.LE";	// file type for little endian platform
#else
	//static constexpr char* HEAD_TEXT_POINT_BLOCK_LIST = "PointBlockList.Unknown";
#endif

	struct SchemaHeader {
		int32_t version;
		uint32_t formatterBitFlags;
		std::string fileHeaderText;

		void WriteTo(std::streambuf* pStreamBuf);
		void ReadFrom(std::streambuf* pStreamBuf);
	};

	struct DocHeader {
		int64_t contentPosition;
		int32_t readableVersion;	//< If the module is older than this value, this file cannot be read.

		void WriteTo(YmBinaryFormatter& formatter);
		void ReadFrom(YmBinaryFormatter& formatter);
	};

	struct PointBlockHeader {
		double localToGlobalMatrix[16];	// row major, pre-multiplied by vector
		double aAabbPoint[2][3];	// aAabbPoint[0] : min point, aAabbPoint[1] : max point.
		int64_t firstBytePos = 0;
		int64_t nImageByte = 0;	// Image size. (not used.)
	};

	struct ExclusiveLodPointList {
		struct Header {
			int32_t version;
			int64_t lodTablePos;
		};

	};


	struct PointType {
		YmVector3f coord;
		YmRgba4b rgba;
	};


	static void WriteVector3d(YmBinaryFormatter& output, const YmVector3d& value);
	static YmVector3d ReadVector3d(YmBinaryFormatter& input);

private:
	YmTngnModel();
	virtual ~YmTngnModel();

	YmTngnModel(const YmTngnModel& other) = delete;
	YmTngnModel(YmTngnModel&& other) noexcept = delete;

	YmTngnModel& operator = (const YmTngnModel& other) = delete;
	YmTngnModel& operator = (YmTngnModel&& other) noexcept = delete;
};

}
