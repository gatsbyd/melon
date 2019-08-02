#include <gtest/gtest.h>
#include "Log.h"
#include "Singleton.h"

#include <iostream>

using namespace std;
using namespace melon;

TEST(TestLog, BaseTest) {
	melon::Logger::setLogLevel(melon::LogLevel::INFO);

	LOG_DEBUG << "debug";
	LOG_INFO << "info";
	LOG_WARN << "warn";
	LOG_ERROR << "error";
	LOG_FATAL << "fatal";
}
