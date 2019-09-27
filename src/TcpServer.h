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
	typedef std::function<void (TcpConnection::Ptr)> ConnectionHanlder;
	TcpServer(const IpAddress& listen_addr, Scheduler* scheduler);
	~TcpServer() {}
	
	void start();
	void setConnectionHandler(ConnectionHanlder&& handler);

private:
	void startAccept();

	IpAddress listen_addr_;
	Socket::Ptr listen_socket_;
	Scheduler* scheduler_;
	ConnectionHanlder connection_handler_;
};

void defualtHandler(TcpConnection::Ptr connection);

}

#endif
