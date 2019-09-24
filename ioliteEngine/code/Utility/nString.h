#pragma once

namespace nString {
	size_t multiByteCharToWideChar(const char* src, wchar_t* dst, size_t src_size);
	void convertToLower(char* dst);
};