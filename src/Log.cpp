#include <assert.h>
#include <iostream>
#include <sstream>
#include <string.h>
#include <unistd.h>

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

LogEvent::LogEvent(Timestamp timestamp, pid_t tid, LogLevel logLevel, 
					const char* file_name,
					int line) 
	: timestamp_(timestamp), tid_(tid), logLevel_(logLevel), 
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
		abort();
	}
}

std::ostream& LogWrapper::getStream() {
	return event_->getStream();
}

Logger::Logger() {
}

void Logger::setLogLevel(LogLevel logLevel) {
	g_logLevel = logLevel;
}

std::string Logger::format(LogEvent::ptr event) {
	//TODO:待优化
	std::ostringstream ss;
	char buf[50];
	time_t sec = event->timestamp_.getSec();
	suseconds_t nsec = event->timestamp_.getUsec();
	struct tm tm;
	localtime_r(&sec, &tm);
	strftime(buf, sizeof buf, "%Y-%m-%d %H:%M:%S", &tm);

	ss << buf << " "
	    << nsec << " "
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
		for (auto& pair : appenders_) {
			pair.second->append(log);
		}
	}
}

void Logger::addAppender(const std::string name, LogAppender::ptr appender) {
	MutexGuard guard(mutex_);
	appenders_.insert(std::make_pair(name, appender));
}

void Logger::delAppender(const std::string name) {
	MutexGuard guard(mutex_);
	appenders_.erase(name);
}

void Logger::clearAppender() {
	MutexGuard guard(mutex_);
	appenders_.clear();
}


void ConsoleAppender::append(const std::string& log) {
	std::cout << log;
}

LogBuffer::LogBuffer(size_t total) 
	:total_(total), available_(total), cur_(0) {
	data_ = new char[total];
}

LogBuffer::~LogBuffer() {
	delete[] data_;
}

size_t LogBuffer::available() const {
	return available_;
}

void LogBuffer::clear() {
	cur_ = 0;
	available_ = total_;
}

void LogBuffer::append(const char* data, size_t len) {
	assert(available_ >= len);
	memcpy(data_ + cur_, data, len);
	cur_ += len;
	available_ -= len;
}

const char* LogBuffer::data() const {
	return data_;
}

size_t LogBuffer::length() const {
	return cur_;
}

AsyncFileAppender::AsyncFileAppender(std::string basename, time_t persist_period) 
	:started_(false), 
	running_(false),
	persist_period_(persist_period), 
	basename_(basename),
	cond_(mutex_) ,
	countdown_latch_(1),
	persit_thread_(std::bind(&AsyncFileAppender::threadFunc, this)),
		cur_buffer_(new LogBuffer()) {
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

		cur_buffer_.reset(new LogBuffer());
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
	std::unique_ptr<LogBuffer> buffer(new LogBuffer());
	std::vector<std::unique_ptr<LogBuffer>> persist_buffers;
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
				cond_.wait_seconds(persist_period_);
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

		//if log is too large, drop it
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
