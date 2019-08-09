#ifndef _MELON_ADDRESS_H_
#define _MELON_ADDRESS_H_

#include <netinet/in.h>
#include <string>

namespace melon {

class IpAddress {
public:
	IpAddress(std::string ip, in_port_t port);
	explicit IpAddress(in_port_t port);
	explicit IpAddress(const struct sockaddr_in& addr);

	//default dtor, copy ctor, assign operation is ok
	
	std::string toString();

private:
	struct sockaddr_in addr_;
};

}

#endif
