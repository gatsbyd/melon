#include "Address.h"
#include "Log.h"
#include "Scheduler.h"
#include "TcpConnection.h"
#include "TcpClient.h"

#include <fstream>
#include <iostream>
#include <stdio.h>
#include <vector>
#include <unordered_map>

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

class Percentile {
public:
	Percentile(std::vector<int>& latencies, int infly) {
		stat << "recv " << latencies.size() << " in-fly " << infly;

		if (!latencies.empty()) {
			std::sort(latencies.begin(), latencies.end());
			int min = latencies.front();
			int max = latencies.back();
			int64_t sum = std::accumulate(latencies.begin(), latencies.end(), static_cast<int64_t>(0));
			int mean = sum / static_cast<int>(latencies.size());
			int median = getPercentile(latencies, 50);
			int p90 = getPercentile(latencies, 90);
			int p99 = getPercentile(latencies, 99);
			stat << " min " << min
				<< " max " << max
				<< " avg " << mean
				<< " median " << median
				<< " p90 " << p90
				<< " p99 " << p99;
		}
	}

	string report() const {
		return stat.str();
	}

private:
	static int getPercentile(const vector<int>& latencies, int percent) {
		assert(!latencies.empty());
		size_t idx = 0;
		if (percent > 0) {
			idx = (latencies.size() * percent + 99) / 100 - 1;
			assert(idx < latencies.size());
		}
		return latencies[idx];
	}
	ostringstream stat;
};

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
		g_scheduler->runEvery(1 * Timestamp::kMicrosecondsPerSecond, std::make_shared<Coroutine>(std::bind(&SudokuClient::calculate, this)));
	}

	void calculate() {
		vector<int> latencies;
		latencies.insert(latencies.end(), latencies_.begin(), latencies_.end());
		latencies_.clear();
		int infly = static_cast<int>(send_time_.size());
		Percentile p(latencies, infly);
		LOG_INFO << p.report();
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
                   //LOG_INFO << "response: " << response;

					//每接收到一个响应再发一个请求
					Timestamp recv_time = Timestamp::now();
					if (verify(response, recv_time)) {
						send(conn, 1);
					} else {

					}
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
		Timestamp now = Timestamp::now();
		for (int i = 0; i < n; ++i) {
			const string& req = (*input_)[count_ % input_->size()];
			conn->write(std::to_string(count_) + ":" + req + "\r\n");
			send_time_[count_] = now;
			++count_;
		}
	}

	bool verify(const string& response, Timestamp recv_time) {
		size_t colon = response.find(":");
		if (colon != string::npos) {
			int id = atoi(response.c_str());
			auto sendtime = send_time_.find(id);
			if (sendtime != send_time_.end()) {
				int64_t latency_us = recv_time.getMicroSecondsFromEpoch() - sendtime->second.getMicroSecondsFromEpoch();	
				latencies_.push_back(static_cast<int>(latency_us));
				send_time_.erase(sendtime);
			} else {
				LOG_ERROR << "Unknown id " << id;
			}
		}
		return true;
	}
private:
	InputPtr input_;
	IpAddress server_addr_;
	int conn_num_;
	int pipelines_;
	int no_delay_;
	int count_;
	vector<TcpConnection::Ptr> connections_;
	unordered_map<int, Timestamp> send_time_;
	vector<int> latencies_;
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
		scheduler.startAsync();
	
		SudokuClient client(input, server_addr, conn_num, pipelines, nodelay);
		scheduler.addTask(std::bind(&SudokuClient::handleClient, &client));

		getchar();
	} else {
		printf("Cannot open %s\n", argv[1]);
	}

	return 0;
}
