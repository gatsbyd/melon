#ifndef _MELON_LOG_H_
#define _MELON_LOG_H_

#include <memory>
#include <vector>
#include <string>
#include <sstream>

namespace melon {

enum class LogLevel;

LogLevel getLogLevel();

#define LOG_DEBUG if getLogLevel() <= LogLevel::DEBUG \
	

class LogEvent {
public:
	LogEvent(pid_t pid, LogLevel logLevel, 
					const std::string file_name,
					int line);
	~LogEvent();
	std::ostream& getStream();
private:
	//todo: time
	pid_t pid_;
	LogLevel logLevel_;
	std::ostringstream content_;
	std::string file_name_;
	int line_;
};


class LogAppender {
public:
	typedef std::shared_ptr<LogAppender> ptr;

	virtual ~LogAppender() {}
	virtual void log(const LogEvent& event) = 0;
protected:
	virtual std::string format(const LogEvent& event);

};

class ConsoleAppender : public LogAppender {
public:
	void log(const LogEvent& event) override;
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

	void log(const LogEvent& event);
	void addAppender(LogAppender::ptr appender);
	void delAppender(LogAppender::ptr appender);
	void clearAppender();

	static void setLogLevel(LogLevel logLevel);
	static LogLevel getLogLevel();

private:
	std::vector<LogAppender::ptr> appenders_;
};

}
#endif

