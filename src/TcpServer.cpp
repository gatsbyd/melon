#include "Log.h"
#include "Scheduler.h"
#include "TcpServer.h"
#include "GoStyleSyntax.h"

#include <assert.h>


namespace melon {

TcpServer::TcpServer(const IpAddress& listen_addr)
	:listen_addr_(listen_addr),
	listen_socket_(Socket::CreateSocket()) {

	listen_socket_.bind(listen_addr_);
}

void TcpServer::start(size_t thread_num) {
	listen_socket_.listen();


	SchedulerSingleton::getInstance()->addTask(std::bind(&TcpServer::onAccept, this), "Accept");
	SchedulerSingleton::getInstance()->start(thread_num);
}

void TcpServer::onAccept() {
	while (true) {
		IpAddress peer_addr;
		int connfd = listen_socket_.accept(peer_addr);

		Socket::Ptr socket = std::make_shared<Socket>(connfd);
		SchedulerSingleton::getInstance()->addTask(std::bind(&TcpServer::handleClient, this, std::make_shared<TcpConnection>(socket, peer_addr)), "Connect");
	}
}

void TcpServer::handleClient(TcpConnection::Ptr connection) {
	LOG_INFO << "new connection, peer addr:" << connection->peerAddr().toString();
}

}
