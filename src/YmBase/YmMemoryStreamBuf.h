#pragma once

#include "YmStreamBuf.h"
#include <deque>

namespace Ymcpp {

class YmMemoryStreamBuf : public YmStreamBuf
{
public:
	YmMemoryStreamBuf() {}
	explicit YmMemoryStreamBuf(size_t bufSize);
	virtual ~YmMemoryStreamBuf();

protected:
	virtual std::streamsize OnReadFromBaseStream(char_type* aBufferByte, std::streamsize nBufferByte);
	virtual std::streamsize OnWriteToBaseStream(const char_type* aBufferByte, std::streamsize nBufferByte);
	virtual pos_type OnSeekBaseStream(off_type offset, ios_base::seekdir way);
private:
	std::deque<char> m_contents;
	size_t m_currentPosition = 0;
};

}
