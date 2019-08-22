#include "LogFile.h"
#include "Log.h"

#include <iostream>
#include <sstream>
#include <stdio.h>
#include <unistd.h>

namespace melon {

LogFile::LogFile(std::string basename) 
	:basename_(basename) {
	std::string file_name = getFileName();
	fp_ = ::fopen(getFileName().c_str(), "w");
}

LogFile::~LogFile() {
	::fclose(fp_);
}

void LogFile::persist(const char* data, size_t length) {
	::fwrite(data, length, 1, fp_);
	bytes_writed_ += length;

	if (bytes_writed_ > kMaxFileSize) {
		flush();
		::fclose(fp_);

		fp_ = ::fopen(getFileName().c_str(), "w");
		bytes_writed_ = 0;
	}
}

void LogFile::flush() {
	::fflush(fp_);
}

std::string LogFile::getFileName() {
	char time[30];
	Timestamp now = Timestamp::now();
	time_t sec = now.getSec();
	struct tm tm;
	localtime_r(&sec, &tm);
	strftime(time, sizeof time, "%Y%m%d-%H%M%S", &tm);
	
	std::stringstream ss;
	ss << basename_ << "." << time << "." << getpid() << ".log";

	return ss.str();
}

}
