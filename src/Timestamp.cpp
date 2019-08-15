#include "Log.h"
#include "Timestamp.h"

#include <errno.h>
#include <sys/time.h>
#include <string.h>

namespace melon {

Timestamp Timestamp::now() {
	struct timeval tv;
	if (gettimeofday(&tv, nullptr)) {
		LOG_ERROR << "gettimeofday:" << strerror(errno);	
	}
	return Timestamp(tv.tv_sec * kMicrosecondsPerSecond + tv.tv_usec);
}

}
