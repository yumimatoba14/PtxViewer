#pragma once

#include "YmStreamBuf.h"
#include <ostream>

namespace Ymcpp {

namespace Details {
	/// <summary>
	/// TODO: This class may not be able to handle multi byte characters correctly.
	/// Base class's buffer may split a multi-byte chracter and it cannot be written correctly.
	/// </summary>
	class DebugOutputStreamBuf : public YmStreamBuf {
	protected:
		virtual std::streamsize OnWriteToBaseStream(const char_type* aBufferByte, std::streamsize nBufferByte);
	private:
		std::vector<char> m_buffer;
	};
}

/// <summary>
/// This stream class writes texts by OutputDebugStringA().
/// </summary>
class YmDebugOutputStream : public std::ostream
{
public:
	YmDebugOutputStream();
	virtual ~YmDebugOutputStream();

private:
	Details::DebugOutputStreamBuf m_streamBuf;
};

}
