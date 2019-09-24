#include "nFile.h"

namespace nFile {
	std::string getRootDirectory() {
		char _path[MAX_PATH];
		GetCurrentDirectory(MAX_PATH, _path);
		return std::string(_path) + "/";
	}
	std::string getResourcePath(const std::string& relativePath) {
		return getRootDirectory() + relativePath;
	}
};