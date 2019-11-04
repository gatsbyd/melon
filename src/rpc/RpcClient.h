#ifndef _MELON_RPC_CLIENT_H_
#define _MELON_RPC_CLIENT_H_

#include "Codec.h"
#include "Log.h"
#include "Scheduler.h"
#include "TcpClient.h"

namespace melon {
namespace rpc {

template <typename T>
class TypeTraits {
	static_assert(std::is_base_of<::google::protobuf::Message, T>::value, "T must be subclass of google::protobuf::Message");
public:
	typedef std::function<void (std::shared_ptr<T>)> ResponseHandler;
};

class RpcClient {
public:
	RpcClient(const IpAddress& server_addr, Scheduler* scheduler)
		:client_(server_addr),
   		scheduler_(scheduler) {
		}
	template <typename T>
	void Call(MessagePtr request, const typename TypeTraits<T>::ResponseHandler& handler);

private:
	TcpClient client_;
	Scheduler* scheduler_;
};

}
}
#endif
