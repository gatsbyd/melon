#ifndef _MELON_RPC_SERVER_H_
#define _MELON_RPC_SERVER_H_

#include "TcpServer.h"
#include "TcpConnection.h"
#include "Address.h"

#include <map>
#include <functional>
#include <google/protobuf/service.h>

namespace melon {
namespace rpc {

class RpcServer : public TcpServer {
public:
	RpcServer(const IpAddress& listen_addr, Scheduler* scheduler)
		:TcpServer(listen_addr, scheduler) {
	setConnectionHandler(std::bind(&RpcServer::handleClient, this, std::placeholders::_1));
}

	void registerService(::google::protobuf::Service* service);

private:
	void handleClient(TcpConnection::Ptr conn);

	std::map<std::string, ::google::protobuf::Service*> services_;
};

}
}

#endif
