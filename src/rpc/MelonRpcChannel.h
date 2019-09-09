#ifndef _MELON_RPC_CHANNEL_H_
#define _MELON_RPC_CHANNEL_H_

#include "TcpConnection.h"

#include <google/protobuf/service.h>

namespace melon {
namespace rpc {
	
class MethodDescriptor;
class RpcController;
class Message;
class Closure;


class MelonRpcChannel : public ::google::protobuf::RpcChannel {
public:
	typedef std::shared_ptr<MelonRpcChannel> Ptr;

	//send request
	void CallMethod(const ::google::protobuf::MethodDescriptor* method, 
					::google::protobuf::RpcController* controller,
					const ::google::protobuf::Message* request,
					::google::protobuf::Message* response, ::google::protobuf::Closure* done) override;


	
	static MelonRpcChannel::Ptr CreateChannel();
private:
	MelonRpcChannel();

	TcpConnection::Ptr conn_;

};


}
}

#endif
