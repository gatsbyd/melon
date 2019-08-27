#include "Log.h"
#include "Scheduler.h"
#include "TcpServer.h"

#include <assert.h>


namespace melon {

TcpServer::TcpServer(const IpAddress& listen_addr, std::shared_ptr<Scheduler> scheduler)
	:listen_addr_(listen_addr),
	listen_socket_(Socket::CreateSocket()),
	scheduler_(scheduler) {

	listen_socket_.bind(listen_addr_);
}

void TcpServer::start(size_t thread_num) {
	listen_socket_.listen();

	scheduler_->addTask(std::bind(&TcpServer::onAccept, this), "Accept");
	scheduler_->start(thread_num);
}

void TcpServer::onAccept() {
	//todo
	while (true) {
		IpAddress peer_addr;
		int connfd = listen_socket_.accept(peer_addr);
		LOG_INFO << "new connection fd:" << connfd;

		Socket::Ptr socket = std::make_shared<Socket>(connfd);
		scheduler_->addTask(std::bind(&TcpServer::handleClient, this, std::make_shared<TcpConnection>(socket, peer_addr)), "Connect");
	}
}

void TcpServer::handleClient(TcpConnection::Ptr connection) {
	//todo:打印对端地址
	connection->shutdown();
}

}
