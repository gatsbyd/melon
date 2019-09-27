#include "TcpServer.h"
#include "TcpClient.h"
#include "Scheduler.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>

using namespace melon;

struct Cmd {
	bool server;
	bool client;
	uint16_t port;
	std::string ip;
	int length;
	int number;

	Cmd()
		:server(false), client(false), port(5000), 
			length(65536), number(1024) {
	}

};

void printUsage(char* app) {
	printf("server side:%s -s port length number\n \
			client size:%s -c ip port length number", app, app);
}

bool parseCmd(int argc, char* argv[], Cmd& cmd) {
	if (argc < 2) {
		printUsage(argv[0]);
		return false;
	}
	if (strcmp("-s", argv[1]) == 0) {
		cmd.server = true;
		if (argc > 2) {
			cmd.port = atoi(argv[2]);
		}
		if (argc > 3) {
			cmd.length = atoi(argv[3]);
		}
		if (argc > 4) {
			cmd.number = atoi(argv[4]);
		}
	} else if (strcmp("-c", argv[1]) == 0) {
		cmd.client = true;
		if (argc > 2) {
			cmd.ip = argv[2];
		} else {
			printUsage(argv[0]);
			return 0;
		}
		if (argc > 3) {
			cmd.port = atoi(argv[2]);
		}
		if (argc > 4) {
			cmd.length = atoi(argv[3]);
		}
		if (argc > 5) {
			cmd.number = atoi(argv[4]);
		}
	} else {
		printUsage(argv[0]);
		return false;
	}
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
