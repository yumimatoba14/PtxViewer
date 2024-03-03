#include "pch.h"
#include "YmDebugOutputStream.h"
#include <Windows.h>

using namespace std;
using namespace Ymcpp;
using namespace Ymcpp::Details;

////////////////////////////////////////////////////////////////////////////////

YmDebugOutputStream::YmDebugOutputStream() : ostream(&m_streamBuf)
{
}

YmDebugOutputStream::~YmDebugOutputStream()
{
}

////////////////////////////////////////////////////////////////////////////////

std::streamsize DebugOutputStreamBuf::OnWriteToBaseStream(const char_type* aBufferByte, std::streamsize nBufferByteIn)
{
	YM_ASSERT(nBufferByteIn <= SIZE_MAX);	// streamsize can be 64bit on x86. So check it.
	size_t nBufferByte = static_cast<size_t>(nBufferByteIn);
	if (m_buffer.size() <= nBufferByte) {
		m_buffer.resize(max<size_t>(nBufferByte + 1, 1024));
	}
	YM_ASSERT(nBufferByte < m_buffer.size());
	strncpy_s(m_buffer.data(), m_buffer.size(), aBufferByte, nBufferByte);
	m_buffer.at(nBufferByte) = '\0';
	::OutputDebugStringA(m_buffer.data());
	return nBufferByte;
}

////////////////////////////////////////////////////////////////////////////////
