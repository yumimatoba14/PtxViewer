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

std::streamsize DebugOutputStreamBuf::OnWriteToBaseStream(const char_type* aBufferByte, std::streamsize nBufferByte)
{
	if (streamsize(m_buffer.size()) <= nBufferByte) {
		m_buffer.resize(max<streamsize>(nBufferByte + 1, 1024));
	}
	YM_ASSERT(nBufferByte < streamsize(m_buffer.size()));
	strncpy_s(m_buffer.data(), m_buffer.size(), aBufferByte, nBufferByte);
	m_buffer.at(nBufferByte) = '\0';
	::OutputDebugStringA(m_buffer.data());
	return nBufferByte;
}

////////////////////////////////////////////////////////////////////////////////
