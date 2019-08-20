#include "TcpServer.h"
#include "Log.h"

#include <unistd.h>

using namespace melon;

class EchoServer : public TcpServer {
public:
	EchoServer(IpAddress& addr, int thread_num) :TcpServer(addr, thread_num) {

	}
protected:
	void handleClient(TcpConnection::Ptr conn) override {
		char buffer[500];
		int n;
		while ((n = conn->read(buffer, sizeof buffer)) > 0) {
			conn->write(buffer, n);
		}
		LOG_DEBUG << "close echo connection";
	}

};

int main() {
	Singleton<Logger>::getInstance()->addAppender("console", LogAppender::ptr(new ConsoleAppender()));

	IpAddress listen_addr(1234);
	EchoServer server(listen_addr, 2);
	server.start();
	return 0;
}
