#ifndef _MELON_RPC_CLIENT_H_
#define _MELON_RPC_CLIENT_H_

#include "Codec.h"
#include "Log.h"
#include "Noncopyable.h"
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

class RpcClient : public Noncopyable {
public:
	typedef std::shared_ptr<RpcClient> Ptr;
	RpcClient(const IpAddress& server_addr, Scheduler* scheduler)
		:client_(server_addr),
   		scheduler_(scheduler) {
		}

	template <typename T>
	inline void Call(MessagePtr request, const typename TypeTraits<T>::ResponseHandler& handler) {
		scheduler_->addTask(std::bind(&RpcClient::handleConnection<T>, this, request, handler));
	}
private:
	template <typename T>
	void handleConnection(MessagePtr request, typename TypeTraits<T>::ResponseHandler handler) {
		TcpConnection::Ptr conn = client_.connect();
		if (conn) {
			LOG_DEBUG << "RpcClient: connect to " << conn->peerAddr().toString() << " success";
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
		} else {
			LOG_DEBUG << "RpcClient: connect to " << conn->peerAddr().toString() << " failed";
		}					
	}

	TcpClient client_;
	Scheduler* scheduler_;
};

}
}
#endif
