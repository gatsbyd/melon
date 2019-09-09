#include "RpcServer.h"

#include <google/protobuf/descriptor.h>

namespace melon {
namespace rpc {

void RpcServer::handleClient(TcpConnection::Ptr conn) {
	//read len
	

}

void RpcServer::registerService(::google::protobuf::Service* service) {
	const ::google::protobuf::ServiceDescriptor* desc = service->GetDescriptor();
	services_[desc->full_name()] = service;
}


}
}
