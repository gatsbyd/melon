#include "Log.h"
#include "TcpClient.h"
#include "Scheduler.h"

#include <unistd.h>

using namespace melon;

class EchoClient : public TcpClient {
public:
	EchoClient(IpAddress server_addr) :TcpClient(server_addr) {}

	void onConnected(TcpConnection::Ptr conn) override {
		int n;
		char send[1024];
		char recv[1024];

		while ( (n = read(STDIN_FILENO, send, sizeof send)) > 0) {
			conn->writen(send, n);

			int n = conn->read(recv, sizeof recv);
			write(STDOUT_FILENO, recv, n);
		}
	}

};

int main(int argc, char* argv[]) {
	if (argc < 3) {
		printf("Usage:%s serverip port\n", argv[0]);
	}
	Singleton<Logger>::getInstance()->addAppender("console", LogAppender::ptr(new ConsoleAppender()));

	IpAddress server_addr(argv[1], atoi(argv[2]));
	EchoClient echo(server_addr);
	echo.start();

	SchedulerSingleton::getInstance()->start();
	return 0;
}
