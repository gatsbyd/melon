#include <iostream>
#include <stdio.h>
#include <vector>
#include <fstream>

#include "Address.h"
#include "Log.h"
#include "Scheduler.h"
#include "TcpClient.h"
#include "Timestamp.h"

using namespace std;
using namespace melon;

Scheduler* g_scheduler;
//int g_conn_num = 1;
int g_finished;
Timestamp g_start;
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
	SudokuClient(const char* input, IpAddress addr, int conn_num)
			:server_addr_(addr), conn_num_(conn_num) {
				ifstream in(input);
				input_ = readInput(in);
	}

	void sudokuClient() {
		TcpClient client(server_addr_);
		for (int i = 0; i < conn_num_; ++i) {
			connections.push_back(client.connect());
		}

		g_start = Timestamp::now();
		for (const TcpConnection::Ptr& connection : connections) {
			g_scheduler->addTask(std::bind(&SudokuClient::handleConnection, this, connection));
		}
	}

	void handleConnection(TcpConnection::Ptr conn) {
		//send request
		LOG_INFO << "start send request, conn_num_ = " << conn_num_;
		for (size_t i = 0; i < input_->size(); ++i) {
			string request = std::to_string(i) + ":" + (*input_)[i] + "\r\n";
			conn->write(request);
			LOG_INFO << "send request:" << request;
		}
		conn->shutdown();
		//receive response
		ssize_t n;
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
				} else if (len > 100) {
					LOG_ERROR << "Bad Response";
					conn->shutdown();
					break;
				} else {
					break;
				}
			}
		}
		++g_finished;
		conn->close();
		if (conn_num_== g_finished) {
			int64_t elapsed = (Timestamp::now() - g_start) / Timestamp::kMicrosecondsPerSecond;
			LOG_INFO << "all connection finished, total " << elapsed << " seconds, "
					<< (1.0 * elapsed / conn_num_) << " seconds per client";
			g_scheduler->stop();
		}
		LOG_INFO << "finish handleConnection";
	}
private:
	InputPtr input_;
	IpAddress server_addr_;
	int conn_num_;
	vector<TcpConnection::Ptr> connections;
};


int main(int argc, char* argv[]) {
	const char* input = nullptr;
	Logger::setLogLevel(LogLevel::INFO);
	Singleton<Logger>::getInstance()->addAppender("console", LogAppender::ptr(new ConsoleAppender()));

	if (argc < 3) {
		printf("Usage:%s input server_ip [connections]\n", argv[0]);
		return 0;
	}
	input = argv[1];
	IpAddress server_addr(argv[2], 5000);
	int conn_num = 1;
	if (argc > 3) {
		conn_num = atoi(argv[3]);
	}

	Scheduler scheduler;
	scheduler.startAsync();
	g_scheduler = &scheduler;
	SudokuClient client(input, server_addr, conn_num);
	scheduler.addTask(std::bind(&SudokuClient::sudokuClient, &client));

	getchar();
	return 0;
}
