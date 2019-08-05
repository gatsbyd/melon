#include "Log.h"
#include "Singleton.h"

#include <iostream>

using namespace std;
using namespace melon;

int main() {
	melon::Logger::setLogLevel(melon::LogLevel::INFO);
	Singleton<Logger>::getInstance()->addAppender("console", LogAppender::ptr(new ConsoleAppender()));
	std::shared_ptr<AsyncFileAppender> file_appender = std::make_shared<AsyncFileAppender>("/tmp/log");	
	file_appender->start();
	Singleton<Logger>::getInstance()->addAppender("file", file_appender);

	LOG_DEBUG << "debug";
	LOG_INFO << "info";
	LOG_WARN << "warn";
	LOG_ERROR << "error";
	//LOG_FATAL << "fatal";
	return 0;
}
