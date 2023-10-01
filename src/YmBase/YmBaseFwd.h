#pragma once

#pragma warning(disable: 26812)	// The enum type 'type-name' is unscoped. Prefer 'enum class' over 'enum' (Enum.3)

#if defined(_WIN64)
#	define YM_IS_32BIT_MODULE (0)
#else
#	define YM_IS_32BIT_MODULE (1)
#endif

#include "YmErrorUtil.h"

// unique_ptr and strings are to be used in declarations.
#include <memory>
#include <string>

namespace Ymcpp {
	class YmBinaryFormatter;

	class YmStdFileCloser {
	public:
		void operator () (FILE* pFile) noexcept {
			if (pFile != nullptr) {
				fclose(pFile);
			}
		}
	};
	using YmUniqueFilePtr = std::unique_ptr<FILE, YmStdFileCloser>;
}
