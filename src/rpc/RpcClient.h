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
	//template <typename T>
	//void Call(MessagePtr request, const typename TypeTraits<T>::ResponseHandler& handler);
template <typename T>
void Call(MessagePtr request, const typename TypeTraits<T>::ResponseHandler& handler) {
	scheduler_->addTask([&]() {
						TcpConnection::Ptr conn = client_.connect();
						if (conn) {
							ProtobufCodec codec(conn);
							codec.send(request);

							Buffer::Ptr buf(new Buffer);
							MessagePtr response;
							ProtobufCodec::ErrorCode errorcode = codec.receive(response);
							if (errorcode == ProtobufCodec::kNoError && response) {
								std::shared_ptr<T> concrete_response = std::static_pointer_cast<T>(response);
								handler(concrete_response);
							} else {
								LOG_ERROR << "receive rpc response error:" << errorcode;
							}

							conn->readUntilZero();
							conn->close();
						}						
					}, "rpc");	
}

private:
	TcpClient client_;
	Scheduler* scheduler_;
};

}
}
#endif
