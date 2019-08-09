#include "Address.h"
#include "Log.h"

#include <arpa/inet.h>
#include <string.h>

namespace melon {

IpAddress::IpAddress(std::string ip, in_port_t port) {
	bzero(&addr_, sizeof addr_);

	addr_.sin_family = AF_INET;
	addr_.sin_port = ::htons(port);
	int s;
	if ((s = ::inet_pton(AF_INET, ip.c_str(), &addr_.sin_addr) <= 0)) {
		if (s == 0) {
			LOG_FATAL << "inet_pton: Not in presentation format";
		} else {
			LOG_FATAL << "inet_pton: " << strerror(errno);
		}
	}
}

IpAddress::IpAddress(in_port_t port) {
	bzero(&addr_, sizeof addr_);
	
	addr_.sin_family = AF_INET;
	addr_.sin_port = ::htons(port);
	addr_.sin_addr.s_addr = ::htonl(INADDR_ANY);

}

IpAddress::IpAddress(const struct sockaddr_in& addr)
	:addr_(addr) {

}

	
std::string IpAddress::toString() const {
	//todo
	return "todo";
}

const struct sockaddr_in* IpAddress::getSockAddr() const {
	return &addr_;
}
}
