#include "pch.h"
#include "YmFilePath.h"

using namespace std;
using namespace Ymcpp;
using namespace ATL;

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
	LPCTSTR pLastSep = _tcsrchr(m_content.c_str(), _T('\\'));
	if (pLastSep != nullptr) {
		if (pLastSep[1] == _T('\0')) {
			return true;
		}
	}
	return false;
}

YmFilePath YmFilePath::GetParentDirectory() const
{
	LPCTSTR pHead = m_content.c_str();
	LPCTSTR pLastSep = _tcsrchr(pHead, _T('\\'));
	if (pLastSep != nullptr) {
		return YmTString(pHead, pLastSep - pHead);
	}
	return YmFilePath();
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
		m_content += _T("\\");
	}
	m_content += pFileName;
	return *this;
}

////////////////////////////////////////////////////////////////////////////////
