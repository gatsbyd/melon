#include "http/HttpServer.h"
#include "HttpConnection.h"

namespace melon {
namespace http {

HttpServer::HttpServer(const IpAddress& listen_addr,
				int thread_num)
		:TcpServer(listen_addr, thread_num) {}

void HttpServer::handleClient(TcpConnection::Ptr conn) {
	HttpConnection::Ptr http_conn = std::make_shared<HttpConnection>(conn);

	auto req = http_conn->recvRequest();

	HttpResponse::Ptr rsp = std::make_shared<HttpResponse>();
	http_conn->sendResponse(rsp);

	//todo
	
		
}

}
}
