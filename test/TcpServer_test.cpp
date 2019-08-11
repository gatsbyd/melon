#include "TcpServer.h"
#include "Log.h"

using namespace melon;

int main() {
	Singleton<Logger>::getInstance()->addAppender("console", LogAppender::ptr(new ConsoleAppender()));

	IpAddress listen_addr(1234);
	TcpServer server(listen_addr);
	server.start();
	return 0;
}
