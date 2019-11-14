#include "http/HttpConnection.h"
#include "TcpServer.h"
#include "Scheduler.h"
#include "Log.h"

#include <stdio.h>

using namespace melon;
using namespace melon::http;

int main() {
	Singleton<Logger>::getInstance()->addAppender("console", LogAppender::ptr(new ConsoleAppender()));

	Scheduler scheduler;
	scheduler.startAsync();
	IpAddress addr(80);
	TcpServer server(addr, &scheduler);
	server.setConnectionHandler([](TcpConnection::Ptr conn) {
					HttpConnection::Ptr http_conn = std::make_shared<HttpConnection>(conn);
					HttpRequest::Ptr request = http_conn->recvRequest();

					HttpResponse::Ptr rsp = std::make_shared<HttpResponse>();
					rsp->setHttpStatus(HttpStatus::OK);
					rsp->setHeader("Content-Length", "5");
					rsp->setContent("hello");
					http_conn->sendResponse(rsp);
				});

	server.start();
	getchar();
	return 0;
}
