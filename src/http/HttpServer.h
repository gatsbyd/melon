#ifndef _MELON_HTTP_SERVER_H_
#define _MELON_HTTP_SERVER_H_

#include "TcpServer.h"

namespace melon {
namespace http {
	
	
class HttpServer : public TcpServer {
public:
	HttpServer(const IpAddress& listen_addr, Scheduler* scheduler);
	~HttpServer() {}

private:
	void handleClient(TcpConnection::Ptr conn);
};


}
}

#endif
