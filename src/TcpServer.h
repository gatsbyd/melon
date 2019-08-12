#ifndef _MELON_TCP_SERVER_H_
#define _MELON_TCP_SERVER_H_

#include "Address.h"
#include "CoroutineScheduler.h"
#include "Socket.h"
#include "SchedulerThread.h"

#include <vector>

namespace melon {
	
class IpAddress;

class TcpServer : public Noncopyable {
public:
	//todo:
	TcpServer(const IpAddress& listen_addr, int thread_num = 0);
	~TcpServer() {}
	
	//todo:
	void start();

protected:
	CoroutineScheduler* selectOneScheduler();
	void onAccept();
	virtual void handleClient(Socket::Ptr socket);

	IpAddress listen_addr_;
	int thread_num_;
	Socket listen_socket_;
	CoroutineScheduler accept_scheduler_;
	std::vector<SchedulerThread::Ptr> thread_pool_;
	std::vector<CoroutineScheduler*> connect_scheduler_;

};

}

#endif
