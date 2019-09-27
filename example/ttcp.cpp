#include "TcpServer.h"
#include "TcpClient.h"
#include "Scheduler.h"

#include <stdio.h>
#include <stdint.h>

using namespace melon;

struct Cmd {
	bool server;
	bool client;
	uint16_t port;
	char* ip;
	int length;
	int number;

	Cmd()
		: server(false), client(false), port(0), ip(nullptr),
			length(0), number(0) {
	}

};

void printUsage(char* app) {
	printf("server side:%s s port length number\n \
			client size:%s c ip port length number", app, app);
}

bool parseCmd(int argc, char* argv[], Cmd& cmd) {

	return true;
}

void transmit(TcpConnection::Ptr conn) {

};

void receive(TcpConnection::Ptr conn) {

}

int main(int argc, char* argv[]) {
	Cmd cmd;
	if (!parseCmd(argc, argv, cmd)) {
		exit(1);
	}

	Scheduler scheduler;
	if (cmd.client) {
		IpAddress serverAddr(cmd.ip, cmd.port);
		TcpClient client(serverAddr);
		scheduler.addTask([&client]() {
							TcpConnection::Ptr conn = client.connect();
							receive(conn);
						});
	} else if (cmd.server) {
		IpAddress addr(cmd.port);
		TcpServer server(addr, &scheduler);
		server.setConnectionHandler(transmit);
		server.start();
	}
	scheduler.start();

	return 0;
}
