#include "nString.h"

namespace nString {
	size_t multiByteCharToWideChar(const char* src, wchar_t* dst, size_t src_size) {
		size_t wlen = 0;
		setlocale(LC_CTYPE, "jpn");
		mbstowcs_s(&wlen, dst, src_size, src, _TRUNCATE);
		return wlen;
	}
	void convertToLower(char* dst) {
		for (; *dst != '\0'; dst++) {
			*dst = tolower(*dst);
		}
	}
};