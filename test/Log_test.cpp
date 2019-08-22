#include "Log.h"
#include "Singleton.h"

#include <iostream>
#include <unistd.h>

using namespace std;
using namespace melon;

int main(int , char* argv[]) {
	//melon::Logger::setLogLevel(melon::LogLevel::INFO);
	//Singleton<Logger>::getInstance()->addAppender("console", LogAppender::ptr(new ConsoleAppender()));
	std::shared_ptr<AsyncFileAppender> file_appender = std::make_shared<AsyncFileAppender>(argv[0]);	
	file_appender->start();
	Singleton<Logger>::getInstance()->addAppender("file", file_appender);

	for (int i = 0; i < 1000000; i++) {
		LOG_DEBUG << "debug";
		LOG_INFO << "info";
		LOG_WARN << "warn";
		LOG_ERROR << "error";
	}
	
	return 0;
}
