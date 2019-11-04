#ifndef _MELON_RPC_CODEC_H_
#define _MELON_RPC_CODEC_H_

#include "TcpConnection.h"

#include <google/protobuf/message.h>

namespace melon {
namespace rpc {

typedef std::shared_ptr<::google::protobuf::Message> MessagePtr;

class ProtobufCodec {
public:
	enum ErrorCode {
		kNoError = 0,
		kInvalidLength,
		kChedksumError,
		kInvalidNameLength,
		kUnknownMessageType,
		kParseError,
		kServerClosed,
	};
	
public:
	ProtobufCodec(TcpConnection::Ptr conn) :conn_(conn) {}
	void send(const MessagePtr& message);
	ErrorCode receive(MessagePtr& message);

private:
	static google::protobuf::Message* createMessage(const std::string& typeName);
	static MessagePtr parse(const char* buf, int len, ErrorCode* errorcode);

	TcpConnection::Ptr conn_;
	const static int kHeaderlen = sizeof(int32_t);
	const static int kMinMessageLen = kHeaderlen + 2 + kHeaderlen; //namelen + typename + checksum
	const static int kMaxMessageLen = 64 * 1024 * 1024;
};

}
}
#endif
