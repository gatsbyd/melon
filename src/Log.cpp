#include <assert.h>
#include <iostream>
#include <sstream>
#include <string.h>

#include "Log.h"

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

std::string Logger::format(LogEvent::ptr event) {
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

void Logger::log(LogEvent::ptr event) {

	std::string log = format(event);
	{
		MutexGuard guard(mutex_);
		for (auto& appender : appenders_) {
			appender->append(log);
		}
	}
}

void Logger::addAppender(LogAppender::ptr appender) {
	MutexGuard guard(mutex_);
	appenders_.push_back(appender);
}

void Logger::delAppender(LogAppender::ptr appender) {
	MutexGuard guard(mutex_);
	for (auto it = appenders_.begin();
					it != appenders_.end(); ++it) {
		if (*it == appender) {
			appenders_.erase(it);
			break;
		}
	}
}

void Logger::clearAppender() {
	MutexGuard guard(mutex_);
	appenders_.clear();
}


void ConsoleAppender::append(std::string log) {
	std::cout << log;
}

Buffer::Buffer(size_t total) 
	:total_(total), available_(total), cur_(0) {
	data_ = new char[total];
	if (!data_) {
		//todo: retry
		LOG_FATAL << "no space to allocate";
	}
}

Buffer::~Buffer() {
	delete[] data_;
}

size_t Buffer::available() {
	return available_;
}

void Buffer::clear() {
	cur_ = 0;
	available_ = total_;
}

void Buffer::append(const char* data, size_t len) {
	assert(available_ >= len);
	memcpy(data_ + cur_, data, len);
	cur_ += len;
	available_ -= len;
}

}
