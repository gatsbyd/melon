#include "Timestamp.h"

#include <iostream>
#include <errno.h>
#include <sys/time.h>
#include <string.h>

namespace melon {

Timestamp Timestamp::now() {
	struct timeval tv;
	if (gettimeofday(&tv, nullptr)) {
		std::cerr << "gettimeofday:" << strerror(errno) << std::endl;
	}
	return Timestamp(tv.tv_sec * kMicrosecondsPerSecond + tv.tv_usec);
}

time_t Timestamp::getSec() const {
	return microseconds_from_epoch_ / kMicrosecondsPerSecond;
}

suseconds_t Timestamp::getUsec() const {
	return microseconds_from_epoch_ % kMicrosecondsPerSecond;
}

}
