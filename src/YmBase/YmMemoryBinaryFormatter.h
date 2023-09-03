#pragma once

#include "YmBinaryFormatter.h"
#include "YmMemoryStreamBuf.h"

namespace Ymcpp {

/// <summary>
/// Convenient binary formatter class to write data to YmMemoryStreamBuf.
/// </summary>
class YmMemoryBinaryFormatter : public YmBinaryFormatter
{
public:
	YmMemoryBinaryFormatter();
	virtual ~YmMemoryBinaryFormatter();

private:
	YmMemoryStreamBuf m_streamBuf;
};

}
