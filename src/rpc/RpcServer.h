#ifndef _MELON_RPC_SERVER_H_
#define _MELON_RPC_SERVER_H_

#include "Address.h"
#include "Codec.h"
#include "Mutex.h"
#include "TcpServer.h"
#include "TcpConnection.h"

#include <assert.h>
#include <map>
#include <functional>
#include <google/protobuf/service.h>

namespace melon {
namespace rpc {

class Callback {
public:
	virtual ~Callback() = default;
	virtual MessagePtr onMessage(const MessagePtr& message) = 0;
};

template <typename T>
class CallbackT : public Callback {
	static_assert(std::is_base_of<::google::protobuf::Message, T>::value, "T must be subclass of google::protobuf::Message");
public:
	typedef std::function<MessagePtr (const std::shared_ptr<T>&)> ConcreteMessageCallback;
	CallbackT(const ConcreteMessageCallback& callback)
			:concrete_callback_(callback) {}
	MessagePtr onMessage(const MessagePtr& message) {
		std::shared_ptr<T> concrete_message = std::static_pointer_cast<T>(message);
		return concrete_callback_(concrete_message);
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

	typedef std::map<const ::google::protobuf::Descriptor*, std::shared_ptr<Callback>> HandlerMap;

	template<typename T>
	void registerRpcHandler(const typename CallbackT<T>::ConcreteMessageCallback& handler) {
		//todo 线程安全
		std::shared_ptr<CallbackT<T> > cp(new CallbackT<T>(handler));
		{
			MutexGuard lock(mutex_);
			handlers_[T::descriptor()] = cp;
		}
	}

private:
	void handleClient(TcpConnection::Ptr conn);

	HandlerMap handlers_;
	Mutex mutex_;
};

}
}

#endif
