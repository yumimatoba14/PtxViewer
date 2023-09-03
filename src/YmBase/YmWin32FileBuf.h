#pragma once

#include "YmStreamBuf.h"
#include "YmWin32.h"

namespace Ymcpp {

////////////////////////////////////////////////////////////////////////////////

class YmWin32FileBuf : public YmStreamBuf
{
public:
	YmWin32FileBuf() {}
	explicit YmWin32FileBuf(YmUniqueHandle&& hFile, size_t bufSize = 1024)
		: YmStreamBuf(bufSize), m_hFile(std::move(hFile))
	{}
	virtual ~YmWin32FileBuf();

public:
	void OpenNewFile(LPCTSTR pFilePath);
	void OpenTempFile(DWORD dwCreationDisposition = CREATE_ALWAYS);

	bool IsOpend() const { return m_hFile.get() != nullptr && m_hFile.get() != INVALID_HANDLE_VALUE; }

	void Close();
	HANDLE GetHandle() const { return m_hFile.get(); }

protected:
	virtual std::streamsize OnReadFromBaseStream(char_type* aBufferByte, std::streamsize nBufferByte);
	virtual std::streamsize OnWriteToBaseStream(const char_type* aBufferByte, std::streamsize nBufferByte);
	virtual pos_type OnSeekBaseStream(off_type offset, ios_base::seekdir way);
private:
	void OnDetachHandle();
private:
	YmUniqueHandle m_hFile;
};

}
