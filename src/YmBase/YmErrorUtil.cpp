#include "pch.h"
#include "YmErrorUtil.h"
#include <exception>
#include "YmDebugOutputStream.h"

using namespace std;
using namespace Ymcpp;

void YmErrorUtil::WriteLog(const char* pMsg, const char* pFile, int srcLine)
{
	YmDebugOutputStream out;
	out << pFile << "(" << srcLine << ") : " << pMsg << endl;
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
