#include "TcpServer.h"
#include "Log.h"
#include "Scheduler.h"

#include <unistd.h>
#include <stdio.h>

using namespace melon;

void handleClient(TcpConnection::Ptr conn){
	conn->setTcpNoDelay(true);
	Buffer::Ptr buffer = std::make_shared<Buffer>();
	while (conn->read(buffer) > 0) {
		conn->write(buffer);
	}

	conn->close();
}


int main(int args, char* argv[]) {
	if (args != 2) {
		printf("Usage: %s threads\n", argv[0]);
		return 0;
	}
	//Logger::setLogLevel(LogLevel::INFO);
	//Singleton<Logger>::getInstance()->addAppender("console", LogAppender::ptr(new ConsoleAppender()));

	IpAddress listen_addr(5000);
	int threads_num = std::atoi(argv[1]);

	Scheduler scheduler(threads_num);
	scheduler.startAsync();
	TcpServer server(listen_addr, &scheduler);
	server.setConnectionHandler(handleClient);
	server.start();

	scheduler.wait();
	return 0;
}
