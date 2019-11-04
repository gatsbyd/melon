#include "RpcClient.h"


namespace melon {
namespace rpc{

template <typename T>
void RpcClient::Call(MessagePtr request, const typename TypeTraits<T>::ResponseHandler& handler) {
	scheduler_->addTask([&]() {
						TcpConnection::Ptr conn = client_.connect();
						if (conn) {
							ProtobufCodec codec(conn);
							codec.send(request);

							Buffer::Ptr buf;
							MessagePtr response;
							ProtobufCodec::ErrorCode errorcode = codec.receive(response);
							if (errorcode == ProtobufCodec::kNoError && response) {
								handler(response);
							} else {
								LOG_ERROR << "receive rpc response error:" << errorcode;
							}

							conn->readUntilZero();
							conn->close();
						}						
					}, "rpc");	
}

}
}
