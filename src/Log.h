#ifndef _MELON_LOG_H_
#define _MELON_LOG_H_

#include <memory>
#include <string>
#include <sstream>
#include <sys/types.h>
#include <time.h>
#include <vector>

#include "Mutex.h"
#include "Singleton.h"
#include "Thread.h"

namespace melon {

enum class LogLevel;
	
class LogEvent {
friend class LogAppender;
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
	virtual void log(LogEvent::ptr event) = 0;
protected:
	virtual std::string format(LogEvent::ptr event);

};

class ConsoleAppender : public LogAppender {
public:
	void log(LogEvent::ptr event) override;
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

	void log(LogEvent::ptr event);
	void addAppender(LogAppender::ptr appender);
	void delAppender(LogAppender::ptr appender);
	void clearAppender();

	static void setLogLevel(LogLevel logLevel);
	static LogLevel getLogLevel();

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
													  melon::LogWrapper(melon::LogEvent::ptr(new melon::LogEvent(time(nullptr), Thread::CurrentThreadTid(), melon::LogLevel::DEBUG, \
									__FILE__, __LINE__))).getStream()

#define LOG_INFO if (melon::Logger::getLogLevel() <= melon::LogLevel::INFO) \
													  melon::LogWrapper(melon::LogEvent::ptr(new melon::LogEvent(time(nullptr), Thread::CurrentThreadTid(), melon::LogLevel::INFO, \
									__FILE__, __LINE__))).getStream()

#define LOG_WARN melon::LogWrapper(melon::LogEvent::ptr(new melon::LogEvent(time(nullptr), Thread::CurrentThreadTid(), melon::LogLevel::WARN, \
									__FILE__, __LINE__))).getStream()

#define LOG_ERROR melon::LogWrapper(melon::LogEvent::ptr(new melon::LogEvent(time(nullptr), Thread::CurrentThreadTid(), melon::LogLevel::ERROR, \
									__FILE__, __LINE__))).getStream()

#define LOG_FATAL melon::LogWrapper(melon::LogEvent::ptr(new melon::LogEvent(time(nullptr), Thread::CurrentThreadTid(), melon::LogLevel::FATAL, \
									__FILE__, __LINE__))).getStream()

#endif

