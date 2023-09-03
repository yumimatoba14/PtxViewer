#pragma once

#include <assert.h>

// This file is included from YmBaseFwd.h.
//#include "YmBaseFwd.h"

namespace Ymcpp {

class YmErrorUtil
{
public:
	static void WriteLog(const char* pMsg, const char* pFile, int srcLine);
	static void ThrowError(const char* pMsg, const char* pFile, int srcLine);
	static void IgnoreError(const char* pMsg, const char* pFile, int srcLine);
};

}

#define YM_WRITE_LOG(msg) YmErrorUtil::WriteLog(msg, __FILE__, __LINE__)
#define YM_THROW_ERROR(msg) YmErrorUtil::ThrowError(msg, __FILE__, __LINE__)
#define YM_IGNORE_ERROR(msg) YmErrorUtil::IgnoreError(msg, __FILE__, __LINE__)

#define YM_NOEXCEPT_BEGIN(section_name) const char* _pSectionName = (section_name); try {
#define YM_NOEXCEPT_END } catch(std::exception& ex) { \
		YM_WRITE_LOG(ex.what()); \
	} catch( ... ) { YM_WRITE_LOG(_pSectionName); }

#define YM_ASSERT(cond) assert(cond)
#define YM_IS_TRUE(cond) if (cond) {} else YM_THROW_ERROR(#cond)

