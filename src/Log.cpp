#include <assert.h>
#include <iostream>
#include <sstream>
#include <string.h>

#include "Log.h"
#include "LogFile.h"

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


void ConsoleAppender::append(const std::string& log) {
	std::cout << log;
}

Buffer::Buffer(size_t total) 
	:total_(total), available_(total), cur_(0) {
	data_ = new char[total];
	if (!data_) {
		//todo: retry
		//LOG_FATAL << "no space to allocate";
	}
}

Buffer::~Buffer() {
	delete[] data_;
}

size_t Buffer::available() const {
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

const char* Buffer::data() const {
	return data_;
}

size_t Buffer::length() const {
	return cur_;
}

AsyncFileAppender::AsyncFileAppender(std::string basename, time_t persist_per_second) 
	:started_(false), 
	running_(false),
	persist_per_second_(persist_per_second), 
	basename_(basename),
	cond_(mutex_) ,
	countdown_latch_(1),
	persit_thread_(std::bind(&AsyncFileAppender::threadFunc, this)),
		cur_buffer_(new Buffer()) {
}

AsyncFileAppender::~AsyncFileAppender() {
	if (started_) {
		stop();	
	}
}

void AsyncFileAppender::append(const std::string& log) {
	MutexGuard guard(mutex_);

	if (cur_buffer_->available() >= log.size()) {
		cur_buffer_->append(log.c_str(), log.size());
	} else {
		buffers_.push_back(std::move(cur_buffer_));

		cur_buffer_.reset(new Buffer());
		cur_buffer_->append(log.c_str(), log.size());
		cond_.notify();
	}
}

void AsyncFileAppender::start() {
	started_ = true;
	running_ = true;
	persit_thread_.start();
	countdown_latch_.wait();
}

void AsyncFileAppender::stop() {
	started_ = false;
	cond_.notify();
	persit_thread_.join();
}

void AsyncFileAppender::threadFunc() {
	std::unique_ptr<Buffer> buffer(new Buffer());
	std::vector<std::unique_ptr<Buffer>> persist_buffers;
	LogFile log_file(basename_);

	countdown_latch_.countDown();

	while (running_) {
		assert(buffer);
		assert(buffer->length() == 0);
		assert(persist_buffers.empty());

		{
			MutexGuard gurd(mutex_);
			//wake up every persist_per_seconds_ or on Buffer is full
			if (buffers_.empty()) {
				cond_.wait_seconds(persist_per_second_);
			}
			if (buffers_.empty() && cur_buffer_->length() == 0) {
				continue;
			}

			buffers_.push_back(std::move(cur_buffer_));

			//reset  cur_buffer_ and buffers_
			persist_buffers.swap(buffers_);
			cur_buffer_ = std::move(buffer);
			cur_buffer_->clear();
			assert(buffers_.empty());
			assert(cur_buffer_);
			assert(cur_buffer_->length() == 0);
		}

		//if log is too large, drop
		if (persist_buffers.size() > 1) {
			std::cerr << "log is too large, drop some" << std::endl;
			persist_buffers.erase(persist_buffers.begin() + 1, persist_buffers.end());
		}

		//persist log
		for (auto& buffer : persist_buffers) {
			log_file.persist(buffer->data(), buffer->length());
		}

		//reset buffer and persist_buffers
		assert(persist_buffers.size() == 1);
		buffer = std::move(persist_buffers[0]);
		buffer->clear();
		persist_buffers.clear();

		log_file.flush();

		if (!started_) {
			MutexGuard guard(mutex_);
			if (cur_buffer_->length() == 0) {
				running_ = false;
			}
		}
	}
	log_file.flush();

	std::cerr << "AsyncFileAppender flush complete" << std::endl;
}

}
