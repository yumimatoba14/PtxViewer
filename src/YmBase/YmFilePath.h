#pragma once

#if defined(_WIN32)
#include "YmWin32.h"
#else
#include "YmBase.h"
#endif

namespace Ymcpp {

class YmFilePath
{
public:
	YmFilePath();
	YmFilePath(const char* path) { SetContent(ATL::CA2T(path)); }
	YmFilePath(const wchar_t* path) { SetContent(ATL::CW2T(path)); }
	YmFilePath(const YmTString& path) { SetContent(path.c_str()); }
	/*virtual*/ ~YmFilePath();

	bool IsEndWithSeparator() const;
	bool IsEmpty() const { return m_content.empty(); }
	std::string ToString() const { return std::string(ATL::CT2A(m_content.c_str())); }
	YmTString ToTString() const { return m_content; }
	YmFilePath GetParentDirectory() const;

	// Function to edit file path.
public:
	/// <summary>
	/// Ensure that IsEndWithSeparator() returns false.
	/// </summary>
	/// <returns></returns>
	YmFilePath& RemoveTailSeparator();
	YmFilePath& AppendFileName(const char* pFileName) { return AppendFileNameImpl(ATL::CA2T(pFileName)); }
	YmFilePath& AppendFileName(const wchar_t* pFileName) { return AppendFileNameImpl(ATL::CW2T(pFileName)); }
	YmFilePath& AppendFileName(const YmTString& fileName) { return AppendFileNameImpl(fileName.c_str()); }

private:
	void SetContent(LPCTSTR pFileName);
	YmFilePath& AppendFileNameImpl(LPCTSTR pFileName);
private:
	YmTString m_content;
};

}
