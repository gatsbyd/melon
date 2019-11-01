#include "RpcServer.h"

#include <google/protobuf/descriptor.h>

namespace melon {
namespace rpc {

void RpcServer::handleClient(TcpConnection::Ptr conn) {
	Codec codec(conn);
	//todo 错误处理
	MessagePtr message = codec.receive();
	::google::protobuf::Descriptor* descriptor = message->GetDescriptor();
	HandlerMap::const_iterator it = handlers_.find(descriptor);

	if (it != handlers_.end()) {
		it->second(message);
	} else {
		//todo
	}

	

}

}
}
