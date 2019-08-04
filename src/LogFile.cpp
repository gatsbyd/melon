#include "LogFile.h"
#include "Log.h"

#include <stdio.h>

namespace melon {

LogFile::LogFile(std::string basename) 
	:basename_(basename) {
	std::string file_name = getFileName();
	fp_ = ::fopen(file_name.c_str(), "w");
}

LogFile::~LogFile() {
	::fclose(fp_);
}

void LogFile::persist(const char* data, size_t length) {
	::fwrite(data, length, 1, fp_);
}

void LogFile::flush() {
	::fflush(fp_);
}

std::string LogFile::getFileName() {
	//todo:
	return basename_;
}

}
