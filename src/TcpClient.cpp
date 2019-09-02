#include "Scheduler.h"
#include "TcpClient.h"
#include  "Log.h"

#include <unistd.h>
#include <sys/socket.h>
#include <string.h>

namespace melon {

const int TcpClient::kMaxRetryDelayMs;

TcpClient::TcpClient(IpAddress server_addr)
	:server_addr_(server_addr) {
}

void TcpClient::start() {
	SchedulerSingleton::getInstance()->addTask(std::bind(&TcpClient::startConnect, this), "Start Connect");
}

void TcpClient::startConnect() {
retry:
	{
		Socket::Ptr sock = std::make_shared<Socket>(Socket::CreateNonBlockSocket());
		int ret = sock->connect(server_addr_);
		if (ret == 0) {
			SchedulerSingleton::getInstance()->addTask(std::bind(&TcpClient::onConnected, this, std::make_shared<TcpConnection>(sock, server_addr_)), "Connect");
		} else if (errno == EAGAIN 
					|| errno == EADDRINUSE 
					|| errno == EADDRNOTAVAIL 
					|| errno == ECONNREFUSED 
					|| errno == ENETUNREACH) {
			retry_delay_ms_ = std::min(retry_delay_ms_ * 2, kMaxRetryDelayMs);
			sleep(retry_delay_ms_);
			goto retry;
		} else {
			LOG_ERROR << "connect error in TcpClinet::startConnect " << strerror(errno);
		}
		
	}
}

void TcpClient::onConnected(TcpConnection::Ptr conn) {
	LOG_DEBUG << "TcpClient: new Tcp Connection created, server address is " << conn->peerAddr().toString();
}

}
