#pragma once

#include "YmBaseFwd.h"
#include <streambuf>
#include <vector>

#define YM_STREAM_BUF_DEBUG (0)

namespace Ymcpp {

class YmStreamBuf : public std::streambuf
{
public:
	using ios_base = std::ios_base;
protected:
	YmStreamBuf() : YmStreamBuf(1024) {}
	YmStreamBuf(size_t bufSize);

public:
	virtual ~YmStreamBuf();

	YmStreamBuf(const YmStreamBuf& other) = delete;
	YmStreamBuf(YmStreamBuf&& other) noexcept = delete;

	YmStreamBuf& operator = (const YmStreamBuf& other) = delete;
	YmStreamBuf& operator = (YmStreamBuf&& other) noexcept = delete;

	// functions for read.
protected:
	virtual std::streamsize xsgetn(char_type* aOutByte, std::streamsize nOutByte);
	virtual int_type underflow();
	virtual int_type uflow();

	// functions for write.
protected:
	virtual std::streamsize xsputn(const char_type* aInByte, std::streamsize nInByte);
	virtual int sync();
	virtual int_type overflow(int_type nextValue);

	// seek functions
protected:
	virtual pos_type seekoff(
		off_type offset, ios_base::seekdir way, ios_base::openmode mode = ios_base::in | ios_base::out
	);
	virtual pos_type seekpos(pos_type pos, ios_base::openmode mode = ios_base::in | ios_base::out);

protected:
	virtual std::streamsize OnReadFromBaseStream(char_type* aBufferByte, std::streamsize nBufferByte);
	virtual std::streamsize OnWriteToBaseStream(const char_type* aBufferByte, std::streamsize nBufferByte);
	virtual pos_type OnSeekBaseStream(off_type offset, ios_base::seekdir way);

protected:
	bool IsReadBufferEnabled() const { return eback() != nullptr; }
	bool IsWriteBufferEnabled() const { return pbase() != nullptr; }
	std::streamsize GetReadBufferSize() const { return m_buffer.size(); }
	void ResetBuffer()
	{
		setp(nullptr, nullptr);
		setg(nullptr, nullptr, nullptr);
	}

private:
	std::streamsize ReadFromBaseStream(char_type* aBufferByte, std::streamsize nBufferByte)
	{
		if (YM_STREAM_BUF_DEBUG) {
			std::fill(aBufferByte, aBufferByte + nBufferByte, -1);
		}
		return OnReadFromBaseStream(aBufferByte, nBufferByte);
	}
	std::streamsize WriteToBaseStream(const char_type* aBufferByte, std::streamsize nBufferByte)
	{
		return OnWriteToBaseStream(aBufferByte, nBufferByte);
	}

private:
	std::vector<char> m_buffer;
};

}
