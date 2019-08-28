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
	while (true) {
		IpAddress peer_addr;
		int connfd = listen_socket_.accept(peer_addr);

		Socket::Ptr socket = std::make_shared<Socket>(connfd);
		scheduler_->addTask(std::bind(&TcpServer::handleClient, this, std::make_shared<TcpConnection>(socket, peer_addr)), "Connect");
	}
}

void TcpServer::handleClient(TcpConnection::Ptr connection) {
	LOG_INFO << "new connection, peer addr:" << connection->peerAddr().toString();
}

}
