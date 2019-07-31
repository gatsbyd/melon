#ifndef _MELON_LOG_H_
#define _MELON_LOG_H_

#include <memory>
#include <vector>
#include <string>
#include <sstream>
#include <sys/types.h>

namespace melon {

enum class LogLevel;
	
class LogEvent {
friend class LogAppender;

public:
	typedef std::shared_ptr<LogEvent> ptr;
	LogEvent(pid_t pid, LogLevel logLevel, 
					const char* file_name,
					int line);
	std::ostream& getStream();

private:
	//todo: time
	pid_t pid_;
	//todo: tid
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
	std::vector<LogAppender::ptr> appenders_;
};

extern LogLevel g_logLevel;
inline LogLevel Logger::getLogLevel() {
	return g_logLevel;
}
}

#define LOG_DEBUG if (melon::Logger::getLogLevel() <= melon::LogLevel::DEBUG) \
													  melon::LogWrapper(melon::LogEvent::ptr(new melon::LogEvent(1, melon::LogLevel::DEBUG, \
									__FILE__, __LINE__))).getStream()


#endif

