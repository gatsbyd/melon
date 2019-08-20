#include "TcpServer.h"
#include "Log.h"

#include <assert.h>

namespace melon {

TcpServer::TcpServer(const IpAddress& listen_addr, int thread_num)
	:listen_addr_(listen_addr),
	thread_num_(thread_num),
	listen_socket_(Socket::CreateSocket(), &accept_scheduler_) {

	listen_socket_.bind(listen_addr_);

	for (int i = 0; i < thread_num; i++) {
		thread_pool_.push_back(SchedulerThread::Ptr(new SchedulerThread()));
	}
}

void TcpServer::start() {
	listen_socket_.listen();

	for (auto& thread : thread_pool_) {
		connect_scheduler_.push_back(thread->startSchedule());
	}
	accept_scheduler_.schedule(std::bind(&TcpServer::onAccept, this), "Accept");
	accept_scheduler_.run();
}

void TcpServer::onAccept() {
	while (true) {
		IpAddress peer_addr;
		int connfd = listen_socket_.accept(peer_addr);
		LOG_INFO << "new connection fd:" << connfd;

		CoroutineScheduler* scheduler = selectOneScheduler();
		assert(scheduler != nullptr);
		scheduler->schedule(std::bind(&TcpServer::handleClient, this, std::make_shared<TcpConnection>(connfd, scheduler)), "Connect");
	}
}

CoroutineScheduler* TcpServer::selectOneScheduler() {
	if (thread_num_ <= 0) {
		return &accept_scheduler_;
	} else {
		static int i = 0;
		return connect_scheduler_[i++ % connect_scheduler_.size()];
	}
}

void TcpServer::handleClient(TcpConnection::Ptr connection) {
	//todo
	connection->shutdown();
}

}
