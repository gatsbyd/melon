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
	bool is_register = false;
	if (errorCode == ProtobufCodec::kNoError && message) {
		const ::google::protobuf::Descriptor* descriptor = message->GetDescriptor();
		{
			MutexGuard lock(mutex_);
			it = handlers_.find(descriptor);
			is_register = it != handlers_.end();
		}
	} else {
		LOG_ERROR << "receive rpc reqeust error: " << errorCode;
		conn->shutdown();
		conn->readUntilZero();
		conn->close();
		return;
	}

	MessagePtr response;
	if (is_register) {
		response = it->second->onMessage(message);
		codec.send(response);
	} else {
		LOG_ERROR << "Unknown message";	
	}
	//TODO:客户端关闭
	conn->shutdown();
	conn->readUntilZero();
	conn->close();
}

}
}
