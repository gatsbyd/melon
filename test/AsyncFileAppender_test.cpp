#include "Log.h"

#include <unistd.h>
#include <iostream>
#include <sys/time.h>

void bench(melon::AsyncFileAppender& appender) {
	std::string oneKbStr(1024, 'a');

	for (int i = 0; i < 1024 * 50; i++) {
		appender.append(oneKbStr);
	}
}

int main() {

	melon::AsyncFileAppender file_appender("/tmp/log");
	file_appender.start();

	bench(file_appender);
	return 0;
}
