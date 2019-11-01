#ifndef _MELON_RPC_CODEC_H_
#define _MELON_RPC_CODEC_H_

#include "TcpConnection.h"

#include <google/protobuf/service.h>

namespace melon {
namespace rpc {

typedef std::shared_ptr<::google::protobuf::Message> MessagePtr;

class Codec {
public:
	Codec(TcpConnection::Ptr conn) :conn_(conn) {}
	void send(const MessagePtr& message);
	MessagePtr receive();

private:
	TcpConnection::Ptr conn_;
};

}
}
#endif
