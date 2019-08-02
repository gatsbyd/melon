#include "Log.h"

#include <iostream>
#include <sstream>

namespace melon {

const char* LogLevelName[] {
	"DEBUG",
	"INFO ",
	"WARN ",
	"ERROR",
	"FATAL" 
};

LogLevel g_logLevel = LogLevel::DEBUG;

LogEvent::LogEvent(time_t time, pid_t tid, LogLevel logLevel, 
					const char* file_name,
					int line) 
	: time_(time), tid_(tid), logLevel_(logLevel), 
		file_name_(file_name), line_(line) {
}

std::ostream& LogEvent::getStream() {
	return content_;
}

LogWrapper::LogWrapper(LogEvent::ptr event)
	:event_(event){
}

LogWrapper::~LogWrapper() {
	//todo: singleton
	Singleton<Logger>::getInstance()->log(event_);
	if (event_->logLevel_ == LogLevel::FATAL) {
		//todo: flush asyncloging
		abort();
	}
}

std::ostream& LogWrapper::getStream() {
	return event_->getStream();
}

Logger::Logger() {
	addAppender(LogAppender::ptr(new ConsoleAppender()));
}

void Logger::setLogLevel(LogLevel logLevel) {
	g_logLevel = logLevel;
}

void Logger::log(LogEvent::ptr event) {
	//todo: thread safe
	for (auto& appender : appenders_) {
		appender->log(event);
	}
}

void Logger::addAppender(LogAppender::ptr appender) {
	//todo: thread safe
	appenders_.push_back(appender);
}

void Logger::delAppender(LogAppender::ptr appender) {
	//todo: thread safe
	for (auto it = appenders_.begin();
					it != appenders_.end(); ++it) {
		if (*it == appender) {
			appenders_.erase(it);
			break;
		}
	}
}

void Logger::clearAppender() {
	//todo: thread safe
	appenders_.clear();
}

std::string LogAppender::format(LogEvent::ptr event) {
	std::ostringstream ss;
	char buf[50];
	time_t time = event->time_;
	struct tm tm;
	localtime_r(&time, &tm);
	strftime(buf, sizeof buf, "%Y-%m-%d %H:%M:%S", &tm);

	ss << buf << " "
		<< event->tid_ << " "
		<< LogLevelName[static_cast<int>(event->logLevel_)] << " "
		<< event->content_.str() << " - "
		<< event->file_name_ << ":"
		<< event->line_ << std::endl;
	return ss.str();
}

void ConsoleAppender::log(LogEvent::ptr event) {
	std::cout << format(event);
}

}
