#include "pch.h"
#include "YmBinaryFormatter.h"
#include <iterator>

using namespace std;
using namespace Ymcpp;

////////////////////////////////////////////////////////////////////////////////

YmBinaryFormatter::YmBinaryFormatter(std::streambuf* pStreamBuf) : m_pStreamBuf(pStreamBuf)
{

}

YmBinaryFormatter::~YmBinaryFormatter()
{
}

////////////////////////////////////////////////////////////////////////////////

/// <summary>
/// Create an object to read/write images which are compatible to memory.
/// </summary>
/// <param name="pStreamBuf">streambuf which is used by returned object.
/// It can be nullptr. A returned object could be used to get flags.</param>
/// <returns></returns>
YmBinaryFormatter YmBinaryFormatter::CreateForMemoryImage(std::streambuf* pStreamBuf)
{
	YmBinaryFormatter result(pStreamBuf);
	result.SetFormatFlag(BIG_ENDIAN, !IsNativeLittleEndianByInt32());
#if defined(_M_IX86) || defined(_M_X64)
	// No other changes are necessary.
#else
#error Not supported environment.
#endif
	return result;
}

////////////////////////////////////////////////////////////////////////////////

void YmBinaryFormatter::SetFormatFlag(FormatFlags flag, bool enable)
{
	if (enable) {
		SetFormatFlags(m_formatFlags | flag);
	}
	else {
		SetFormatFlags(m_formatFlags & (~flag));
	}
	OnFormatFlagsChanged(flag);
}

////////////////////////////////////////////////////////////////////////////////

int64_t YmBinaryFormatter::GetCurrentPosition() const
{
	if (m_pStreamBuf) {
		return static_cast<int64_t>(m_pStreamBuf->pubseekoff(0, ios_base::cur));
	}
	return 0;
}

void YmBinaryFormatter::SetCurrentPosition(int64_t pos)
{
	if (m_pStreamBuf) {
		m_pStreamBuf->pubseekpos(pos);
	}
}

void YmBinaryFormatter::Flush()
{
	if (m_pStreamBuf) {
		m_pStreamBuf->pubsync();
	}
}

void YmBinaryFormatter::WriteBytes(const char* aByte, size_t nByte)
{
	if (m_pStreamBuf) {
		streamsize nPut = m_pStreamBuf->sputn(aByte, nByte);
		YM_IS_TRUE(nPut == nByte);
	}
}

void YmBinaryFormatter::ReadBytes(char* aByte, size_t nByte)
{
	YM_IS_TRUE(m_pStreamBuf);
	YM_ASSERT(aByte != nullptr);
	streamsize nRead = m_pStreamBuf->sgetn(aByte, nByte);
	YM_IS_TRUE(nRead == nByte);
}

void YmBinaryFormatter::ReadBytes(size_t nByte)
{
	YM_IS_TRUE(m_pStreamBuf);
	size_t nRemainingByte = nByte;
	while (0 < nRemainingByte) {
		char aBufferByte[1024];
		size_t nBufferByte = min(nRemainingByte, sizeof(aBufferByte));
		streamsize nRead = m_pStreamBuf->sgetn(aBufferByte, nBufferByte);
		YM_IS_TRUE(nRead == nBufferByte);
		nRemainingByte -= static_cast<size_t>(nRead);
	}
}

////////////////////////////////////////////////////////////////////////////////

void YmBinaryFormatter::WriteString(const std::string& value)
{
	WriteInt64(value.size());
	WriteBytes(value.c_str(), value.size());
}

std::string YmBinaryFormatter::ReadString()
{
	int64_t nChar = ReadInt64();
	YM_IS_TRUE(nChar < SIZE_MAX);
	size_t nCharSize = static_cast<size_t>(nChar);
	vector<char> buffer(nCharSize + 1);
	ReadBytes(buffer.data(), nCharSize);
	buffer[nCharSize] = '\0';
	return string(buffer.data(), nCharSize);
}

static void CopyStreamBufWithBuffer(streambuf* pTo, streambuf* pFrom, int64_t nByte, char* aBufByte, int64_t bufSize)
{
	int64_t nRemainingByte = nByte;
	while (0 < nRemainingByte) {
		int64_t nWriteByte = min(nRemainingByte, bufSize);
		size_t nRead = static_cast<size_t>(pFrom->sgetn(aBufByte, nWriteByte));
		if (nRead == 0) {
			YM_THROW_ERROR("Failed to read data from pBuf.");
		}
		size_t nWrite = static_cast<size_t>(pTo->sputn(aBufByte, nRead));
		YM_IS_TRUE(nWrite == nRead);
		nRemainingByte -= nRead;
	}
}

static void CopyStreamBufLargeBuf(streambuf* pTo, streambuf* pFrom, int64_t nByte)
{
	constexpr int BUF_SIZE = 1 << 16;
	vector<char> bufByte(BUF_SIZE);
	CopyStreamBufWithBuffer(pTo, pFrom, nByte, bufByte.data(), BUF_SIZE);
}

static void CopyStreamBufSmallBuf(streambuf* pTo, streambuf* pFrom, int64_t nByte)
{
	constexpr int BUF_SIZE = 1 << 12;
	char aBufByte[BUF_SIZE];
	CopyStreamBufWithBuffer(pTo, pFrom, nByte, aBufByte, BUF_SIZE);
}

static void CopyStreamBuf(streambuf* pTo, streambuf* pFrom, int64_t nByte)
{
	YM_IS_TRUE(pTo);
	YM_IS_TRUE(pFrom);
	if (nByte < 0x100000) {
		CopyStreamBufSmallBuf(pTo, pFrom, nByte);
	}
	else {
		CopyStreamBufLargeBuf(pTo, pFrom, nByte);
	}
}

void YmBinaryFormatter::WriteFromStreamBuf(std::streambuf* pBuf, int64_t nByte)
{
	CopyStreamBuf(m_pStreamBuf, pBuf, nByte);
}

void YmBinaryFormatter::ReadToStreamBuf(std::streambuf* pBuf, int64_t nByte)
{
	CopyStreamBuf(pBuf, m_pStreamBuf, nByte);
}

////////////////////////////////////////////////////////////////////////////////

bool YmBinaryFormatter::IsNativeLittleEndianByInt32()
{
	union {
		int i;
		char aByte[sizeof(int)];
	} x;
	x.i = 1;
	YM_IS_TRUE(x.aByte[0] == 1 || x.aByte[sizeof(int) - 1] == 1);
	return x.aByte[0] == 1;
}

bool YmBinaryFormatter::IsNativeLittleEndianByDouble()
{
	union {
		double d;
		char aByte[sizeof(double)];
	} x;
	x.d = 1;
	const char firstByte = 0x3F;
	YM_IS_TRUE(x.aByte[0] == firstByte || x.aByte[sizeof(double) - 1] == firstByte);
	return x.aByte[sizeof(double) - 1] == firstByte;
}

////////////////////////////////////////////////////////////////////////////////

void YmBinaryFormatter::OnFormatFlagsChanged(FormatFlags flag)
{
	if (flag & BIG_ENDIAN) {
#if defined(_M_IX86) || defined(_M_X64)
		m_isReverseEndian = GetFormatFlag(BIG_ENDIAN);
#else
#error not implemented
#endif
	}
}

////////////////////////////////////////////////////////////////////////////////
