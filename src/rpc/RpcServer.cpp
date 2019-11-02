#include "RpcServer.h"
#include "log.h"

#include <google/protobuf/descriptor.h>

namespace melon {
namespace rpc {

void RpcServer::handleClient(TcpConnection::Ptr conn) {
	Codec codec(conn);
	//todo 错误处理
	MessagePtr message = codec.receive();
	::google::protobuf::Descriptor* descriptor = message->GetDescriptor();
	HandlerMap::const_iterator it = handlers_.find(descriptor);

	MessagePtr response;
	if (it != handlers_.end()) {
	 response = it->second->onMessage(message);
	} else {
		//todo
		LOG_INFO << "unknown message";	
	}
	codec.send(response);
	conn->shutdown();
	conn->readUntilZero();
	conn->close();
}

}
}
