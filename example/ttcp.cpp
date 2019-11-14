#include "TcpServer.h"
#include "TcpClient.h"
#include "Scheduler.h"
#include "Timestamp.h"
#include "Log.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>

using namespace melon;

struct Message {
	int32_t number;
	int32_t length;
} __attribute__ ((__packed__));

struct Payload {
	char data[0];
};

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
	printf("server side:%s -s port\nclient size:%s -c ip port length number\n", app, app);
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
	} else if (strcmp("-c", argv[1]) == 0) {
		cmd.client = true;
		if (argc > 2) {
			cmd.ip = argv[2];
		} else {
			printUsage(argv[0]);
			return 0;
		}
		if (argc > 3) {
			cmd.port = atoi(argv[3]);
		}
		if (argc > 4) {
			cmd.length = atoi(argv[4]);
		}
		if (argc > 5) {
			cmd.number = atoi(argv[5]);
		}
	} else {
		printUsage(argv[0]);
		return false;
	}
	return true;
}

void transmit(TcpConnection::Ptr conn, Cmd cmd) {
	Timestamp start = Timestamp::now();
	//send header message
	struct Message message = {0, 0};
	message.length = htonl(cmd.length);
	message.number = htonl(cmd.number);
	if (conn->writen(&message, sizeof(message)) != sizeof(message)) {
		LOG_FATAL << "short write occur when send message";
	}
	//fill payload
	std::unique_ptr<Payload, std::function<void (Payload*)>> payload(static_cast<Payload*>(::malloc(cmd.length)), 
					[](Payload* payload){
						printf("free payload\n");
						::free(payload);
					});
	for (int i = 0; i < cmd.length; ++i) {
		payload->data[i] = "1234567890"[i % 10];
	}
	
	//send payload
	for (int i = 0; i < cmd.number; ++i) {
		if (conn->writen(payload.get(), cmd.length) != static_cast<ssize_t>(cmd.length)) {
			LOG_FATAL << "short write occur when send payload";
		}
		int ack = 0;
		if (conn->readn(&ack, sizeof(ack)) != sizeof(ack)) {
			LOG_FATAL << "short read occur when read ack";
		}
		ack = ntohl(ack);
		LOG_INFO << "send " << cmd.length << " bytes payload, receive ack:" << ack;
		if (ack != 	cmd.length) {
			LOG_FATAL << "ack = " << ack << ", cmd.length = " << cmd.length;
		}
	}
	double total_mb = 1.0 * cmd.length * cmd.number / 1024 / 1024;
	time_t elapsed = Timestamp::now().getSec() - start.getSec();
	printf("%ld seconds, %.3f Mb, %.3f MiB/s\n", elapsed, total_mb, total_mb / elapsed);
};

void receive(TcpConnection::Ptr conn) {
	//receive header message
	struct Message message = {0, 0};
	if (conn->readn(&message, sizeof(message)) != sizeof(message)) {
		LOG_FATAL << "short read occur when read message";
	}
	message.number = ntohl(message.number);
	message.length = ntohl(message.length);
	LOG_INFO << "message.number = " << message.number << ", message.length = " << message.length;

	if (message.length >= 10 * 1024 * 1024) {
		return;
	}

	std::unique_ptr<Payload, std::function<void (Payload*)>> payload(static_cast<Payload*>(::malloc(message.length)), 
					[](Payload* payload){
						printf("free payload\n");
						::free(payload);
					});
	int readn;
	for (int i = 0; i < message.number; ++i) {
		if ((readn = conn->readn(payload.get(), message.length)) != message.length) {
			LOG_FATAL << "short read occur when read payload";
		}
		LOG_INFO << "receive " << readn << " tytes payload, send ack:" << readn;
		readn = htonl(readn);
		if (conn->writen(&readn, sizeof(readn)) != sizeof(readn)) {
			LOG_FATAL << "short write occur when write ack";
		}
	}
}

int main(int argc, char* argv[]) {
	Logger::setLogLevel(LogLevel::INFO);
	Singleton<Logger>::getInstance()->addAppender("console", LogAppender::ptr(new ConsoleAppender()));
	
	Cmd cmd;
	if (!parseCmd(argc, argv, cmd)) {
		exit(1);
	}

	Scheduler scheduler;
	scheduler.startAsync();
	if (cmd.client) {
		IpAddress server_addr(cmd.ip, cmd.port);
		TcpClient client(server_addr);
		scheduler.addTask([&client, &cmd]() {
							TcpConnection::Ptr conn = client.connect();
							transmit(conn, cmd);
						});
		getchar();
	} else if (cmd.server) {
		IpAddress addr(cmd.port);
		TcpServer server(addr, &scheduler);
		server.setConnectionHandler(receive);
		server.start();
		getchar();
	}
	return 0;
}
