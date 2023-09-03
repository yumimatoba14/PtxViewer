#pragma once

#include "YmBaseFwd.h"
#include <Windows.h>
#include <tchar.h>
#include <intsafe.h>

// To use CA2T(), the following 2 header files are needed.
#include <atlconv.h>
#include <atlexcept.h>

namespace Ymcpp {

struct YmWin32HandleCloser {
	void operator ()(HANDLE h) noexcept {
		if (h != INVALID_HANDLE_VALUE) {
			::CloseHandle(h);
		}
	}
};
struct YmWin32FindFileCloser {
	void operator ()(HANDLE h) noexcept {
		if (h != INVALID_HANDLE_VALUE) {
			::FindClose(h);
		}
	}
};
using YmUniqueHandle = std::unique_ptr<void, YmWin32HandleCloser>;
using YmUniqueFindFileHandle = std::unique_ptr<void, YmWin32FindFileCloser>;

#ifdef _UNICODE
using YmTString = std::wstring;
#else
using YmTString = std::string;
#endif
}
