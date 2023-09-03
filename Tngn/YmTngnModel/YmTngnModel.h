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

	static constexpr int32_t CURRENT_FILE_VERSION = 0;

	struct FileHeader {
		int32_t version;
		uint32_t formatterBitFlags;
		std::string fileHeaderText;
		int64_t contentPosition;

		void WriteTo(std::streambuf* pStreamBuf);
		void ReadFrom(std::streambuf* pStreamBuf);
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

private:
	YmTngnModel();
	virtual ~YmTngnModel();

	YmTngnModel(const YmTngnModel& other) = delete;
	YmTngnModel(YmTngnModel&& other) noexcept = delete;

	YmTngnModel& operator = (const YmTngnModel& other) = delete;
	YmTngnModel& operator = (YmTngnModel&& other) noexcept = delete;
};

}
