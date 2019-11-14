#include "TcpServer.h"
#include "Log.h"
#include "Scheduler.h"

#include <unistd.h>
#include <stdio.h>

using namespace melon;

void handleClient(TcpConnection::Ptr conn){
	LOG_INFO << "new connection, peer addr:" << conn->peerAddr().toString();
	char buffer[500];
	int n;
	while ((n = conn->read(buffer, sizeof buffer)) > 0) {
		conn->write(buffer, n);
	}
	LOG_DEBUG << "close echo connection";
}


int main() {
	Singleton<Logger>::getInstance()->addAppender("console", LogAppender::ptr(new ConsoleAppender()));

	IpAddress listen_addr(1234);

	Scheduler scheduler(2);
	scheduler.startAsync();
	TcpServer server(listen_addr, &scheduler);
	server.setConnectionHandler(handleClient);
	server.start();

	getchar();

	return 0;
}
