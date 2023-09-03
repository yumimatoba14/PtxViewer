#include "pch.h"
#include "YmWin32FileBuf.h"

using namespace std;
using namespace Ymcpp;

////////////////////////////////////////////////////////////////////////////////

YmWin32FileBuf::~YmWin32FileBuf()
{
	YM_NOEXCEPT_BEGIN("YmWin32FileBuf::~YmWin32FileBuf");
	OnDetachHandle();
	YM_NOEXCEPT_END;
}

////////////////////////////////////////////////////////////////////////////////

void YmWin32FileBuf::OpenNewFile(LPCTSTR pFilePath)
{
	Close();

	DWORD dwDesiredAccess = FILE_GENERIC_READ | GENERIC_WRITE;
	DWORD dwCreationDisposition = CREATE_ALWAYS;
	DWORD dwFlagsAndAttributes = FILE_ATTRIBUTE_NORMAL;
	m_hFile.reset(::CreateFile(
		pFilePath, dwDesiredAccess, 0, nullptr,
		dwCreationDisposition, dwFlagsAndAttributes, nullptr
	));
	if (m_hFile.get() == INVALID_HANDLE_VALUE) {
		YM_THROW_ERROR("CreateFile");
	}
}

static wstring MakeTempFilePath()
{
	TCHAR aTempDir[_MAX_PATH];
	DWORD ret = ::GetTempPath(_MAX_PATH, aTempDir);
	if (ret == 0 || _MAX_PATH < ret) {
		YM_THROW_ERROR("GetTempPath");
	}

	TCHAR aTempFilePath[_MAX_PATH];
	ret = ::GetTempFileName(aTempDir, _T("pnt"), 0, aTempFilePath);
	if (ret == 0) {
		YM_THROW_ERROR("GetTempFileName");
	}
	return aTempFilePath;
}

void YmWin32FileBuf::OpenTempFile(DWORD dwCreationDisposition /*= CREATE_ALWAYS*/)
{
	Close();

	wstring tempFilePath = MakeTempFilePath();
	m_hFile.reset(::CreateFile(
		tempFilePath.c_str(), FILE_GENERIC_READ | GENERIC_WRITE, 0, nullptr,
		dwCreationDisposition, FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE, nullptr
	));
	if (m_hFile.get() == INVALID_HANDLE_VALUE) {
		YM_THROW_ERROR("CreateFile");
	}
}

void YmWin32FileBuf::Close()
{
	OnDetachHandle();
	ResetBuffer();
	m_hFile.reset(nullptr);
}

////////////////////////////////////////////////////////////////////////////////

std::streamsize YmWin32FileBuf::OnReadFromBaseStream(char_type* aBufferByte, std::streamsize nBufferByte)
{
	YM_ASSERT(sizeof(streamsize) == 8);
	streamsize nRemainingByte = nBufferByte;
	streamsize nReadByte = 0;
	while (0 < nRemainingByte) {
		const DWORD BUF_SIZE_MAX = DWORD_MAX;
		DWORD bufSize = static_cast<DWORD>(nRemainingByte);
		if (BUF_SIZE_MAX < nRemainingByte) {
			bufSize = BUF_SIZE_MAX;
		}
		DWORD nReadByteLocal = 0;
		BOOL isOk = ::ReadFile(m_hFile.get(), aBufferByte + nReadByte, bufSize, &nReadByteLocal, nullptr);
		if (!isOk) {
			YM_THROW_ERROR("ReadFile");
		}
		nReadByte += nReadByteLocal;
		nRemainingByte -= nReadByteLocal;
		if (nReadByteLocal != bufSize) {
			break;
		}
	}
	return nReadByte;
}

std::streamsize YmWin32FileBuf::OnWriteToBaseStream(const char_type* aBufferByte, std::streamsize nBufferByte)
{
	YM_ASSERT(sizeof(streamsize) == 8);
	streamsize nRemainingByte = nBufferByte;
	streamsize nWrittenByte = 0;
	while (0 < nRemainingByte) {
		const DWORD BUF_SIZE_MAX = DWORD_MAX;
		DWORD bufSize = static_cast<DWORD>(nRemainingByte);
		if (BUF_SIZE_MAX < nRemainingByte) {
			bufSize = BUF_SIZE_MAX;
		}
		DWORD nWritten = 0;
		BOOL isOk = ::WriteFile(m_hFile.get(), aBufferByte + nWrittenByte, bufSize, &nWritten, nullptr);
		if (!isOk) {
			YM_THROW_ERROR("WriteFile");
		}
		nWrittenByte += nWritten;
		nRemainingByte -= nWritten;
		if (nWritten != bufSize) {
			break;
		}
	}
	return nWrittenByte;
}

YmWin32FileBuf::pos_type YmWin32FileBuf::OnSeekBaseStream(off_type offset, ios_base::seekdir way)
{
	DWORD method;
	switch (way) {
	case ios_base::beg: method = FILE_BEGIN; break;
	case ios_base::cur: method = FILE_CURRENT; break;
	case ios_base::end: method = FILE_END; break;
	default:
		YM_THROW_ERROR("Not supported seekdir.");
	}

	LARGE_INTEGER offsetLI;
	offsetLI.QuadPart = offset;
	LARGE_INTEGER newPosLI;
	BOOL isOk = ::SetFilePointerEx(m_hFile.get(), offsetLI, &newPosLI, method);
	if (!isOk) {
		YM_THROW_ERROR("SetFilePointerEx");
	}
	return streampos(newPosLI.QuadPart);
}

////////////////////////////////////////////////////////////////////////////////

void YmWin32FileBuf::OnDetachHandle()
{
	if (IsWriteBufferEnabled()) {
		if (pubsync()) {
			YM_THROW_ERROR("pubsync");
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
