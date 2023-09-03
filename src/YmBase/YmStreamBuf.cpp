#include "pch.h"
#include "YmStreamBuf.h"

using namespace std;
using namespace Ymcpp;

////////////////////////////////////////////////////////////////////////////////

YmStreamBuf::YmStreamBuf(size_t bufSize) : m_buffer(bufSize)
{
}

YmStreamBuf::~YmStreamBuf()
{
}

////////////////////////////////////////////////////////////////////////////////

std::streamsize YmStreamBuf::xsgetn(char_type* aOutByte, std::streamsize nOutByte)
{
	streamsize nCopied = 0;
	while (nCopied < nOutByte) {
		streamsize nRequiredByte = nOutByte - nCopied;
		streamsize nByteInBuf = egptr() - gptr();
		if (0 < nByteInBuf) {
			// Copy data from buffer.
			// gbump() supports only int. So maximum number here shall be INT_MAX.
			const int BUMP_SIZE_MAX = INT_MAX;
			streamsize nCopy = min(nByteInBuf, nRequiredByte);
			if (BUMP_SIZE_MAX < nCopy) {
				nCopy = BUMP_SIZE_MAX;
			}
			traits_type::copy(aOutByte + nCopied, gptr(), static_cast<int>(nCopy));
			gbump(static_cast<int>(nCopy));
			nCopied += nCopy;
		}
		else {
			if (nRequiredByte < GetReadBufferSize()) {
				// Read to the buffer and copy from buffer at the next turn.
				if (traits_type::eq_int_type(underflow(), traits_type::eof())) {
					// undeflow() didn't work.
					break;
				}
			}
			else {
				// Read to the output buffer directly.
				streamsize nCopy = ReadFromBaseStream(aOutByte + nCopied, nRequiredByte);
				nCopied += nCopy;
				if (nCopy < nRequiredByte) {
					// If data is not read enough, return from this function call.
					break;
				}
			}
		}
	}
	return nCopied;
}

// get a character from stream, but don't point past it.
YmStreamBuf::int_type YmStreamBuf::underflow()
{
	// This function may enable input buffer. So output buffer must have been disabled.
	YM_IS_TRUE(!IsWriteBufferEnabled());
	YM_ASSERT(gptr() == egptr());
	if (m_buffer.empty()) {
		// sgetc() may call underflow() in this case.
		// Add a buffer for single character to hold current position.
		// In any case, small buffer is not used by xsgetn() or xsputn().
		m_buffer.resize(1);
	}
	streamsize streamBufferSize = GetReadBufferSize();
	streamsize nRead = ReadFromBaseStream(m_buffer.data(), streamBufferSize);
	if (nRead == 0) {
		return traits_type::eof();
	}
	setg(m_buffer.data(), m_buffer.data(), m_buffer.data() + nRead);
	return traits_type::to_int_type(*gptr());
}

// get a character from stream, point past it.
YmStreamBuf::int_type YmStreamBuf::uflow()
{
	if (m_buffer.empty()) {
		// implementation for sbumpc() in the case of no buffer.
		YM_ASSERT(gptr() == egptr());
		char_type nextByte = 0;
		streamsize nRead = ReadFromBaseStream(&nextByte, 1);
		if (nRead == 0) {
			return traits_type::eof();
		}
		return traits_type::to_int_type(nextByte);
	}
	else {
		// Use implementation of the base class which uses underflow().
		return streambuf::uflow();
	}
}

////////////////////////////////////////////////////////////////////////////////

std::streamsize YmStreamBuf::xsputn(const char_type* aInByte, std::streamsize nInByte)
{
	// This function may enable output buffer. So input buffer must have been disabled.
	YM_IS_TRUE(!IsReadBufferEnabled());

	if (pbase() == epptr() && !m_buffer.empty()) {
		// enable buffer
		setp(m_buffer.data(), m_buffer.data() + m_buffer.size());
		if (YM_STREAM_BUF_DEBUG) {
			std::fill(m_buffer.begin(), m_buffer.end(), -1);
		}
	}

	if (pbase() < pptr() && pptr() == epptr()) {
		// There are some bytes in the buffer and the buffer is full.
		// Write the buffer to file in order to try to make some space in the buffer.
		streamsize nByteInBuf = pptr() - pbase();
		YM_ASSERT(nByteInBuf <= SIZE_MAX);
		streamsize nWritten = WriteToBaseStream(pbase(), nByteInBuf);
		YM_ASSERT(0 <= nWritten);
		if (nWritten == 0) {
			return 0;	// failed to write.
		}
		if (nWritten < nByteInBuf) {
			size_t nRemainingByte = static_cast<size_t>(nByteInBuf - nWritten);
			traits_type::move(pbase(), pbase() + nWritten, nRemainingByte);
			setp(pbase(), pbase() + nRemainingByte, epptr());
		}
		else {
			setp(pbase(), epptr());
		}
	}

	streamsize bufferSpace = epptr() - pptr();
	if (nInByte <= bufferSpace) {
		// If there are enough space in the buffer, write data to the buffer.
		// pbump() supports only int. So maximum number here shall be INT_MAX.
		const int BUMP_SIZE_MAX = INT_MAX;
		streamsize numRemaining = nInByte;
		streamsize numCopied = 0;
		while (BUMP_SIZE_MAX < numRemaining) {
			traits_type::copy(pptr(), aInByte + numCopied, BUMP_SIZE_MAX);
			pbump(BUMP_SIZE_MAX);
			numCopied += BUMP_SIZE_MAX;
			numRemaining -= BUMP_SIZE_MAX;
		}
		traits_type::copy(pptr(), aInByte + numCopied, static_cast<int>(numRemaining));
		pbump(static_cast<int>(numRemaining));
		return nInByte;
	}
	// There is no enough space in the buffer, write buffer data at first, and write aInChar after that.

	if (pubsync()) {
		return 0;	// pubsync() was failed.
	}

	return WriteToBaseStream(aInByte, nInByte);
}

int YmStreamBuf::sync()
{
	if (pbase() < pptr()) {
		// write buffer to file.
		streamsize nByteInBuf = pptr() - pbase();
		streamsize nWritten = WriteToBaseStream(pbase(), nByteInBuf);
		if (nWritten < nByteInBuf) {
			return -1;	// failed to write.
		}
		setp(pbase(), epptr());
	}
	return 0;
}

YmStreamBuf::int_type YmStreamBuf::overflow(int_type nextValue)
{
	if (!traits_type::eq_int_type(nextValue, traits_type::eof())) {
		char_type c = traits_type::to_char_type(nextValue);
		streamsize nWritten = sputn(&c, 1);
		if (nWritten == 0) {
			return traits_type::eof();
		}
	}
	return traits_type::not_eof(nextValue);
}

////////////////////////////////////////////////////////////////////////////////

/// <summary>
/// 
/// </summary>
/// <param name="offset"></param>
/// <param name="way"></param>
/// <param name="mode"></param>
/// <returns></returns>
/// This function disables both input and output buffers.
YmStreamBuf::pos_type YmStreamBuf::seekoff(
	off_type offset, ios_base::seekdir way, ios_base::openmode mode /*= ios_base::in | ios_base::out*/
)
{
	if (pbase() < pptr()) {
		if (pubsync()) {
			YM_THROW_ERROR("pubsync() failed.");
		}
	}
	off_type fileOffset = offset;
	if (way == ios_base::cur) {
		if (IsReadBufferEnabled()) {
			// maintain offset value so as to cancel bytes
			// which are in the buffer and have not been read, yet.
			ptrdiff_t numByteInBuffer = egptr() - gptr();
			YM_ASSERT(0 <= numByteInBuffer);
			fileOffset -= numByteInBuffer;
		}
	}
	ResetBuffer();

	return OnSeekBaseStream(fileOffset, way);
}

YmStreamBuf::pos_type YmStreamBuf::seekpos(
	pos_type pos, ios_base::openmode mode /*= ios_base::in | ios_base::out*/
)
{
	return seekoff(pos, ios_base::beg, mode);
}

////////////////////////////////////////////////////////////////////////////////

streamsize YmStreamBuf::OnReadFromBaseStream(char_type* aBufferByte, streamsize nBufferByte)
{
	YM_ASSERT("Not implemented.");
	return 0;
}

streamsize YmStreamBuf::OnWriteToBaseStream(const char_type* aBufferByte, streamsize nBufferByte)
{
	YM_ASSERT("Not implemented.");
	return 0;
}

YmStreamBuf::pos_type YmStreamBuf::OnSeekBaseStream(off_type offset, ios_base::seekdir way)
{
	YM_ASSERT("Not implemented.");
	return pos_type(off_type(-1));
}

////////////////////////////////////////////////////////////////////////////////
