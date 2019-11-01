#ifndef _MELON_RPC_SERVER_H_
#define _MELON_RPC_SERVER_H_

#include "Address.h"
#include "Codec.h"
#include "TcpServer.h"
#include "TcpConnection.h"

#include <map>
#include <functional>
#include <google/protobuf/service.h>

namespace melon {
namespace rpc {

class Callback {
public:
	virtual ~Callback() = default;
	virtual void onMessage(const MessagePtr& message) = 0;
};

template <typename T>
class CallbackT : public Callback {
public:
	CallbackT(const ConcreteMessageCallback& callback)
			:concrete_callback_(callback) {}
	typedef std::function<void (const std::shared_ptr<T>&)> ConcreteMessageCallback;
	void onMessage(const MessagePtr& message) {
		//todo 将message转T然后回调concrete_callback_
	}
private:
	ConcreteMessageCallback concrete_callback_;
};

class RpcServer : public TcpServer {
public:
	RpcServer(const IpAddress& listen_addr, Scheduler* scheduler)
		:TcpServer(listen_addr, scheduler) {
	setConnectionHandler(std::bind(&RpcServer::handleClient, this, std::placeholders::_1));
}

	//todo
	typedef std::function<void (MessagePtr)> RpcHandler;
	typedef std::map<const ::google::protobuf::Descriptor*, RpcHandler> HandlerMap;

	template<typename T>
	void registerRpcHandler(const RpcHandler& handler) {
		//todo 线程安全
		handlers_[T::descriptor()] = handler;
	}

private:
	void handleClient(TcpConnection::Ptr conn);

	HandlerMap handlers_;
};

}
}

#endif
