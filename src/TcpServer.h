#ifndef _MELON_TCP_SERVER_H_
#define _MELON_TCP_SERVER_H_

#include "Address.h"
#include "Noncopyable.h"
#include "Socket.h"
#include "TcpConnection.h"

#include <vector>

namespace melon {

class Scheduler;

class TcpServer : public Noncopyable {
public:
	TcpServer(const IpAddress& listen_addr, Scheduler* scheduler);
	~TcpServer() {}
	
	void start();

protected:
	virtual void handleClient(TcpConnection::Ptr connection);

private:
	void startAccept();

	IpAddress listen_addr_;
	Socket listen_socket_;
	Scheduler* scheduler_;
};

}

#endif
