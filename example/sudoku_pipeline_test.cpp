#include "Address.h"
#include "Log.h"
#include "Scheduler.h"
#include "TcpConnection.h"
#include "TcpClient.h"

#include <fstream>
#include <iostream>
#include <stdio.h>
#include <vector>

using namespace melon;
using namespace std;

Scheduler* g_scheduler;

const int kCells = 81;

typedef std::vector<string> Input;
typedef std::shared_ptr<Input> InputPtr;

InputPtr readInput(std::istream& in) {
    InputPtr input(new Input);
    std::string line;
    while (getline(in, line)) {
        if (line.size() == static_cast<size_t>(kCells)) {
            input->push_back(line.c_str());
        }
    }
    return input;
}


class SudokuClient {
public:
	SudokuClient(InputPtr input, const IpAddress& addr, int conn_num,
					int pipelines, bool no_delay)
   		:input_(input),
		server_addr_(addr),
		conn_num_(conn_num),
		pipelines_(pipelines),
		no_delay_(no_delay),
		count_(0) {

	}

	void handleClient() {
		TcpClient client(server_addr_);
		for (int i = 0; i < conn_num_; ++i) {
			connections_.push_back(client.connect());
		}
		for (const TcpConnection::Ptr& connection : connections_) {
			g_scheduler->addTask(std::bind(&SudokuClient::handleConnection, this, connection));
		}
	}

	void handleConnection(TcpConnection::Ptr conn) {
		//先发送pipeline个请求
		send(conn, pipelines_);
		
		ssize_t n = 0;
		Buffer::Ptr buffer = std::make_shared<Buffer>();
		while ((n = conn->read(buffer)) > 0) {
           size_t len = buffer->readableBytes();
           while (len > kCells + 2) {
               const char* crlf = buffer->findCRLF();
               if (crlf) {
                   string response (buffer->peek(), crlf);
                   buffer->retrieveUntil(crlf + 2);
                   len = buffer->readableBytes();
                   LOG_INFO << "response: " << response;

					//接收到一个响应再发一个请求
					send(conn, 1);
               } else if (len > 100) {
                   LOG_ERROR << "Bad Response";
                   conn->shutdown();
                   break;
               } else {
                   break;
               }
           }
		}

	}

	void send(TcpConnection::Ptr conn, int n) {

		for (int i = 0; i < n; ++i) {
			const string& req = (*input_)[count_ % input_->size()];
			conn->write(std::to_string(count_) + ":" + req);
			++count_;
		}
	}
private:
	InputPtr input_;
	IpAddress server_addr_;
	int conn_num_;
	int pipelines_;
	int no_delay_;
	int count_;
	vector<TcpConnection::Ptr> connections_;

};

int main(int argc, char* argv[]) {
	Logger::setLogLevel(LogLevel::INFO);
	Singleton<Logger>::getInstance()->addAppender("console", LogAppender::ptr(new ConsoleAppender()));

	int conn_num = 1;
	int pipelines = 1;
	bool nodelay = false;
	IpAddress server_addr;
	switch (argc) {
		case 6:
			nodelay = string(argv[5]) == "-n";
		case 5:
			pipelines = atoi(argv[4]);
		case 4:
			conn_num = atoi(argv[3]);
		case 3:
			server_addr = IpAddress(argv[2], 5000);
		case 2:
			break;
		default:
			printf("Usage: %s input server_ip [connections] [pipelines] [-n]\n", argv[0]);
		return 0;
	}

	std::ifstream in(argv[1]);
	if (in) {
		InputPtr input(readInput(in));
		Scheduler scheduler;
		g_scheduler = &scheduler;
	
		SudokuClient client(input, server_addr, conn_num, pipelines, nodelay);
		scheduler.addTask(std::bind(&SudokuClient::handleClient, &client));

		scheduler.start();
	} else {
		printf("Cannot open %s\n", argv[1]);
	}

	return 0;
}
