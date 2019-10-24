#include <iostream>
#include <stdio.h>
#include <vector>
#include <fstream>

#include "Address.h"
#include "Log.h"
#include "Scheduler.h"
#include "TcpClient.h"

using namespace std;
using namespace melon;

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
	SudokuClient(const char* input, IpAddress addr, int conn)
			:server_addr_(addr),
			conn_(conn) {
				ifstream in(input);
				input_ = readInput(in);
			}
	void sudokuClient() {
		TcpClient client(server_addr_);
		for (int i = 0; i < conn_; ++i) {
			connections.push_back(client.connect());
		}

		for (const TcpConnection::Ptr& connection : connections) {
			g_scheduler->addTask(std::bind(&SudokuClient::receiveResponse, this, connection));
		}

		for (const TcpConnection::Ptr& connection : connections) {
			for (size_t i = 0; i < input_->size(); ++i) {
				connection->write(std::to_string(i) + ":" + (*input_)[i] + "\r\n");
			}
		}

	}
	void receiveResponse(TcpConnection::Ptr conn) {
		ssize_t n;
		Buffer::Ptr buffer = std::make_shared<Buffer>();
		while ((n = conn->read(buffer)) > 0) {
			size_t len = buffer->readableBytes();
			while (len > kCells + 2) {
				const char* crlf = buffer->findCRLF();

				if (crlf) {
					try {
						string response (buffer->peek(), crlf);
						buffer->retrieveUntil(crlf + 2);
						len = buffer->readableBytes();
						LOG_INFO << "response: " << response;
					} catch (length_error err) {
						LOG_ERROR << crlf - buffer->peek();
						throw err;
					}
				} else if (len > 100) {
					LOG_INFO << "Bad Response";
					conn->shutdown();
				}
			}
		}
		//conn->close();
	}
private:
	InputPtr input_;
	IpAddress server_addr_;
	int conn_;
	vector<TcpConnection::Ptr> connections;
};


int main(int argc, char* argv[]) {
	int conn = 1;
	const char* input = nullptr;
	Logger::setLogLevel(LogLevel::INFO);
	Singleton<Logger>::getInstance()->addAppender("console", LogAppender::ptr(new ConsoleAppender()));

	if (argc < 3) {
		printf("Usage:%s input server_ip [connections]\n", argv[0]);
		return 0;
	}
	input = argv[1];
	IpAddress server_addr(argv[2], 5000);
	if (argc > 3) {
		conn = atoi(argv[3]);
	}

	Scheduler scheduler;
	g_scheduler = &scheduler;
	SudokuClient client(input, server_addr, conn);
	scheduler.addTask(std::bind(&SudokuClient::sudokuClient, client));

	scheduler.start();

	return 0;
}
