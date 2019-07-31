#include "Log.h"

#include <iostream>
#include <sstream>

namespace melon {

LogLevel g_logLevel = LogLevel::DEBUG;

LogEvent::LogEvent(pid_t pid, LogLevel logLevel, 
					const std::string file_name,
					int line) {
}

LogEvent::~LogEvent() {
	//todo: singleton
	Logger log;
	log.log(*this);
}

std::ostream& LogEvent::getStream() {
	return content_;
}

void Logger::setLogLevel(LogLevel logLevel) {
	g_logLevel = logLevel;
}

inline LogLevel Logger::getLogLevel() {
	return g_logLevel;
}

void Logger::log(const LogEvent& event) {
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

std::string LogAppender::format(const LogEvent& event) {
	std::ostringstream ss;
	ss << "a" << "b";
	return "";
}

void ConsoleAppender::log(const LogEvent& event) {
	std::cout << format(event);
}

}
