#include "Scheduler.h"
#include "TcpClient.h"
#include  "Log.h"

#include <unistd.h>
#include <sys/socket.h>
#include <string.h>

namespace melon {

const int TcpClient::kMaxRetryDelayMs;

TcpClient::TcpClient(const IpAddress& server_addr)
	:server_addr_(server_addr) {
}

TcpConnection::Ptr TcpClient::connect() {
retry:
	{
		Socket::Ptr sock = Socket::CreateTcp();
		sock->SetNonBlockAndCloseOnExec();

		int ret = sock->connect(server_addr_);
		if (ret == 0) {
			return std::make_shared<TcpConnection>(sock, server_addr_);
		} else if (errno == EAGAIN 
					|| errno == EADDRINUSE 
					|| errno == EADDRNOTAVAIL 
					|| errno == ECONNREFUSED 
					|| errno == ENETUNREACH) {
			retry_delay_ms_ = std::min(retry_delay_ms_ * 2, kMaxRetryDelayMs);
			sleep(retry_delay_ms_);
			goto retry;
		} else {
			LOG_ERROR << "connect error in TcpClinet::startConnect, " << strerror(errno);
			return nullptr;
		}
		
	}
}

}
