#include "pch.h"
#include "YmFilePath.h"

using namespace std;
using namespace Ymcpp;
using namespace ATL;

#define PATH_SEP_CHAR_SLASH _T('/')
#define PATH_SEP_CHAR_WIN _T('\\')
#define EXT_SEP_CHAR _T('.')
#define NULL_CHAR (_T('\0'))

#if defined(_WIN32)
#define PATH_SEP_STR _T("\\")
#else
#define PATH_SEP_STR _T("/")
#endif

////////////////////////////////////////////////////////////////////////////////

YmFilePath::YmFilePath()
{
}

YmFilePath::~YmFilePath()
{
}

////////////////////////////////////////////////////////////////////////////////

bool YmFilePath::IsEndWithSeparator() const
{
	LPCTSTR pLastSep = _tcsrchr(m_content.c_str(), PATH_SEP_CHAR_SLASH);
	if (pLastSep != nullptr) {
		if (pLastSep[1] == NULL_CHAR) {
			return true;
		}
	}
#if defined(_WIN32)
	pLastSep = _tcsrchr(m_content.c_str(), PATH_SEP_CHAR_WIN);
	if (pLastSep != nullptr) {
		if (pLastSep[1] == NULL_CHAR) {
			return true;
		}
	}
#endif
	return false;
}

static LPCTSTR GetLastPathSeparator(LPCTSTR pHead)
{
	LPCTSTR pLastSep = _tcsrchr(pHead, PATH_SEP_CHAR_SLASH);
#if defined(_WIN32)
	{
		LPCTSTR pLastSep2 = _tcsrchr(pHead, PATH_SEP_CHAR_WIN);
		if (pLastSep == nullptr || pLastSep < pLastSep2) {
			pLastSep = pLastSep2;
		}
	}
#endif
	return pLastSep;
}

YmFilePath YmFilePath::GetParentDirectory() const
{
	LPCTSTR pHead = m_content.c_str();
	LPCTSTR pLastSep = GetLastPathSeparator(pHead);
	if (pLastSep != nullptr) {
		return YmTString(pHead, pLastSep - pHead);
	}
	return YmFilePath();
}

YmFilePath YmFilePath::GetFileName() const
{
	if (m_content.empty()) {
		return YmTString();
	}
	LPCTSTR pLastSep = GetLastPathSeparator(m_content.c_str());
	if (pLastSep != nullptr) {
		return YmTString(pLastSep + 1);
	}
	return m_content;
}

std::string YmFilePath::GetExtension() const
{
	using namespace ATL;
	YmTString fileName = GetFileName().ToTString();
	LPCTSTR pLastDot = _tcsrchr(fileName.c_str(), EXT_SEP_CHAR);
	if (pLastDot == nullptr) {
		return string();
	}
	if (pLastDot == fileName.c_str()) {
		// If the file name start with dot(.) and it is the only dot, that name shall not have a file extension.
		return string();
	}
	return string(CT2A(pLastDot + 1));
}

////////////////////////////////////////////////////////////////////////////////

YmFilePath& YmFilePath::RemoveTailSeparator()
{
	if (IsEndWithSeparator()) {
		YM_ASSERT(0 < m_content.size());
		m_content.resize(m_content.size() - 1);

		YM_ASSERT(!IsEndWithSeparator());
	}
	return *this;
}

////////////////////////////////////////////////////////////////////////////////

void YmFilePath::SetContent(LPCTSTR pFileName)
{
	if (pFileName == nullptr) {
		m_content.clear();
	}
	else {
		m_content = pFileName;
	}
}

YmFilePath& YmFilePath::AppendFileNameImpl(LPCTSTR pFileName)
{
	YM_IS_TRUE(pFileName != nullptr);
	if (!IsEndWithSeparator()) {
		m_content += PATH_SEP_STR;
	}
	m_content += pFileName;
	return *this;
}

YmFilePath& YmFilePath::ReplaceExtensionImpl(LPCTSTR pNewExt)
{
	YM_IS_TRUE(pNewExt == nullptr || pNewExt[0] != EXT_SEP_CHAR);
	YmTString newSuffix;
	if (pNewExt != nullptr && pNewExt[0] != NULL_CHAR) {
		newSuffix = EXT_SEP_CHAR;
		newSuffix += pNewExt;
	}
	LPCTSTR pBegin = m_content.c_str();
	LPCTSTR pFileNameBegin = pBegin;
	LPCTSTR pSeparator = GetLastPathSeparator(pBegin);
	if (pSeparator != nullptr) {
		pFileNameBegin = pSeparator + 1;
	}
	LPCTSTR pDot = _tcsrchr(pFileNameBegin, EXT_SEP_CHAR);
	if (pDot != nullptr) {
		if (pDot == pFileNameBegin) {
			pDot = nullptr;
		}
	}
	if (pDot != nullptr) {
		m_content.replace(pDot - pBegin, m_content.size() - (pDot - pBegin), newSuffix.c_str());
	}
	else {
		if (!newSuffix.empty()) {
			m_content += newSuffix;
		}
	}
	return *this;
}

////////////////////////////////////////////////////////////////////////////////
