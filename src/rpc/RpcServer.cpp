#include "RpcServer.h"
#include "Log.h"

#include <google/protobuf/descriptor.h>

namespace melon {
namespace rpc {

void RpcServer::handleClient(TcpConnection::Ptr conn) {
	ProtobufCodec codec(conn);
	//todo 错误处理
	MessagePtr message;
	ProtobufCodec::ErrorCode errorCode = codec.receive(message);
	HandlerMap::const_iterator it;
	if (errorCode == ProtobufCodec::kNoError && message) {
		const ::google::protobuf::Descriptor* descriptor = message->GetDescriptor();
		it = handlers_.find(descriptor);
	} else {
		LOG_ERROR << "receive rpc reqeust error: " << errorCode;
		conn->shutdown();
		conn->readUntilZero();
		conn->close();
	}

	MessagePtr response;
	if (it != handlers_.end()) {
		response = it->second->onMessage(message);
		codec.send(response);
	} else {
		LOG_ERROR << "Unknown message";	
	}
	conn->shutdown();
	conn->readUntilZero();
	conn->close();
}

}
}
