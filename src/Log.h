#ifndef _MELON_LOG_H_
#define _MELON_LOG_H_

#include <memory>
#include <string>
#include <sstream>
#include <sys/types.h>
#include <time.h>
#include <vector>

#include "Condition.h"
#include "Mutex.h"
#include "Singleton.h"
#include "Thread.h"

namespace melon {

enum class LogLevel;
	
class LogEvent {
friend class Logger;
friend class LogWrapper;

public:
	typedef std::shared_ptr<LogEvent> ptr;
	LogEvent(time_t time, pid_t tid, LogLevel logLevel, 
					const char* file_name,
					int line);
	std::ostream& getStream();

private:
	time_t time_;
	pid_t tid_;
	//todo: fiber id
	LogLevel logLevel_;
	std::ostringstream content_;
	std::string file_name_;
	int line_;
};

class LogWrapper {
public:
	LogWrapper(LogEvent::ptr event);
	~LogWrapper();
	
	std::ostream& getStream();
private:
	LogEvent::ptr event_;
};

class LogAppender {
public:
	typedef std::shared_ptr<LogAppender> ptr;

	virtual ~LogAppender() {}
	virtual void append(std::string log) = 0;

};

class ConsoleAppender : public LogAppender {
public:
	void append(std::string log) override;
};

class Buffer : public Noncopyable {
public:
	Buffer(size_t total = 1024 * 1024 * 30);
	~Buffer();

	void clear();
	void append(const char* data, size_t len);
	const char* data() const;
	size_t length() const;
	size_t available() const;

private:
	char* data_;
	const size_t total_;
	size_t available_;
	size_t cur_;
};

class AsyncFileAppender : public LogAppender {
public:
	AsyncFileAppender(std::string basename, time_t persist_per_second = 3);
	~AsyncFileAppender();
	void append(std::string log) override;
	void start();
	void stop();

private:
	void threadFunc();
	
	bool started_;
	time_t persist_per_second_;
	std::string basename_;
	Mutex mutex_;
	Condition cond_;
	Thread persit_thread_;
	std::unique_ptr<Buffer> cur_buffer_;
	std::vector<std::unique_ptr<Buffer>> buffers_;
};

enum class LogLevel {
	DEBUG,
	INFO,
	WARN,
	ERROR,
	FATAL, 
};

class Logger {
public:
	Logger();
	virtual ~Logger() {};

	void log(LogEvent::ptr event);
	void addAppender(LogAppender::ptr appender);
	void delAppender(LogAppender::ptr appender);
	void clearAppender();

	static void setLogLevel(LogLevel logLevel);
	static LogLevel getLogLevel();

protected:
	virtual std::string format(LogEvent::ptr event);

private:
	Mutex mutex_;
	std::vector<LogAppender::ptr> appenders_;
};

extern LogLevel g_logLevel;
inline LogLevel Logger::getLogLevel() {
	return g_logLevel;
}

template class Singleton<Logger>;

}

#define LOG_DEBUG if (melon::Logger::getLogLevel() <= melon::LogLevel::DEBUG) \
													  melon::LogWrapper(melon::LogEvent::ptr(new melon::LogEvent(time(nullptr), melon::Thread::CurrentThreadTid(), melon::LogLevel::DEBUG, \
									__FILE__, __LINE__))).getStream()

#define LOG_INFO if (melon::Logger::getLogLevel() <= melon::LogLevel::INFO) \
													  melon::LogWrapper(melon::LogEvent::ptr(new melon::LogEvent(time(nullptr), melon::Thread::CurrentThreadTid(), melon::LogLevel::INFO, \
									__FILE__, __LINE__))).getStream()

#define LOG_WARN melon::LogWrapper(melon::LogEvent::ptr(new melon::LogEvent(time(nullptr), melon::Thread::CurrentThreadTid(), melon::LogLevel::WARN, \
									__FILE__, __LINE__))).getStream()

#define LOG_ERROR melon::LogWrapper(melon::LogEvent::ptr(new melon::LogEvent(time(nullptr), melon::Thread::CurrentThreadTid(), melon::LogLevel::ERROR, \
									__FILE__, __LINE__))).getStream()

#define LOG_FATAL melon::LogWrapper(melon::LogEvent::ptr(new melon::LogEvent(time(nullptr), melon::Thread::CurrentThreadTid(), melon::LogLevel::FATAL, \
									__FILE__, __LINE__))).getStream()

#endif

