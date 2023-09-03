#include "pch.h"
#include "YmErrorUtil.h"
#include <exception>

using namespace std;
using namespace Ymcpp;

void YmErrorUtil::WriteLog(const char* pMsg, const char* pFile, int srcLine)
{
}

void YmErrorUtil::ThrowError(const char* pMsg, const char* pFile, int srcLine)
{
	WriteLog(pMsg, pFile, srcLine);
	throw exception(pMsg);
}

void YmErrorUtil::IgnoreError(const char* pMsg, const char* pFile, int srcLine)
{
	WriteLog(pMsg, pFile, srcLine);
}
