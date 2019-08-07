#ifndef _MELON_TCP_SERVER_H_
#define _MELON_TCP_SERVER_H_

#include "CoroutineScheduler.h"

#include <vector>

namespace melon {

class TcpServer {
public:
	//todo:
	TcpServer(std::string ip, int port, int thread_num);
	
	//todo:
	void start() {
		//todo:将监听描述符加到scheduler中
		accept_scheduler_->run();
	}

private:
	std::string ip_;
	int port_;
	int thread_num_;
	CoroutineScheduler* accept_scheduler_;
	std::vector<CoroutineScheduler*> connect_scheduler_;
	
};

}

#endif
