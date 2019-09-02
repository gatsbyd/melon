#ifndef _MELON_TCP_CLINET_H_
#define _MELON_TCP_CLINET_H_

#include "TcpConnection.h"

namespace melon {

class TcpClient {
public:
	TcpClient(IpAddress server_addr);
	virtual ~TcpClient() {}

	void start();

protected:
	virtual void onConnected(TcpConnection::Ptr conn);

private:
	void startConnect();

	static const int kMaxRetryDelayMs = 15 * 1000;
	static const int kInitRetryDelayMs = 500;

	IpAddress server_addr_;
	int retry_delay_ms_ = kInitRetryDelayMs;
};

}

#endif
