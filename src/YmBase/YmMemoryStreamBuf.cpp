#include "pch.h"
#include "YmMemoryStreamBuf.h"

using namespace std;
using namespace Ymcpp;

////////////////////////////////////////////////////////////////////////////////

YmMemoryStreamBuf::YmMemoryStreamBuf(size_t bufSize) : YmStreamBuf(bufSize)
{
}

YmMemoryStreamBuf::~YmMemoryStreamBuf()
{
}

////////////////////////////////////////////////////////////////////////////////

std::streamsize YmMemoryStreamBuf::OnReadFromBaseStream(char_type* aBufferByte, streamsize nBufferByte)
{
	YM_ASSERT(m_currentPosition <= m_contents.size());
	size_t readByte = static_cast<size_t>(
		min<streamsize>(m_contents.size() - m_currentPosition, nBufferByte));
	auto itBegin = m_contents.begin() + m_currentPosition;
	copy(itBegin, itBegin + readByte, aBufferByte);
	m_currentPosition += readByte;
	return readByte;
}

std::streamsize YmMemoryStreamBuf::OnWriteToBaseStream(const char_type* aBufferByte, streamsize nBufferByte)
{
	streamsize newEnd = m_currentPosition + nBufferByte;
	YM_IS_TRUE(newEnd <= SIZE_MAX);
	const size_t sizeNewEnd = static_cast<size_t>(newEnd);
	if (m_contents.size() < sizeNewEnd) {
		m_contents.resize(sizeNewEnd);
	}
	YM_ASSERT(sizeNewEnd <= m_contents.size());
	copy(aBufferByte, aBufferByte + nBufferByte, m_contents.begin() + m_currentPosition);
	m_currentPosition = sizeNewEnd;
	return nBufferByte;
}

YmMemoryStreamBuf::pos_type YmMemoryStreamBuf::OnSeekBaseStream(off_type offset, ios_base::seekdir way)
{
	off_type newPos = -1;
	if (way == ios_base::beg) {
		newPos = offset;
	}
	else if (way == ios_base::cur) {
		newPos = m_currentPosition + offset;
	}
	else if (way == ios_base::end) {
		newPos = m_contents.size() + offset;
	}
	else {
		YM_THROW_ERROR("Not supported way value.");
	}

	YM_IS_TRUE(0 <= newPos);
	if (SIZE_MAX < newPos) {
		YM_THROW_ERROR("Invalid new position.");
	}
	const size_t sizeNewPos = static_cast<size_t>(newPos);
	YM_IS_TRUE(sizeNewPos <= m_contents.size());
	m_currentPosition = sizeNewPos;
	return pos_type(m_currentPosition);
}

////////////////////////////////////////////////////////////////////////////////
