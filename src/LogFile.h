#ifndef _MELON_LOG_FILE_H_
#define _MELON_LOG_FILE_H_

#include "Noncopyable.h"

#include <string>


namespace melon {

class LogFile : public Noncopyable {
public:
	LogFile(std::string basename);
	~LogFile();

	void persist(const char* data, size_t length);
	void flush();

private:
	std::string getFileName();

	std::string basename_;
	FILE* fp_;
};

}

#endif
