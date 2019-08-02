#include "Log.h"
#include "Singleton.h"

#include <iostream>

using namespace std;
using namespace melon;

int main() {
	melon::Logger::setLogLevel(melon::LogLevel::INFO);

	LOG_DEBUG << "debug";
	LOG_INFO << "info";
	LOG_WARN << "warn";
	LOG_ERROR << "error";
	LOG_FATAL << "fatal";
	return 0;
}
