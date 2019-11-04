#include "Codec.h"
#include "Buffer.h"

#include <zlib.h>

namespace melon {
namespace rpc {

void ProtobufCodec::send(const MessagePtr& message) {
	(void)message;
}

ProtobufCodec::ErrorCode ProtobufCodec::receive(MessagePtr& message) {
	Buffer::Ptr buf;
	while (conn_->read(buf) > 0) {
		if (buf->readableBytes() >= kHeaderlen + kMinMessageLen) {
			const int32_t len = buf->peekInt32();
			if (len > kMaxMessageLen || len < kMinMessageLen) {
				return kInvalidLength;
			} else if (buf->readableBytes() >= len + kHeaderlen) {
				ErrorCode errorcode = kNoError;
				message = parse(buf->peek() + kHeaderlen, len, &errorcode);
				return errorcode;
			}
		}
	}
}

int32_t asInt32(const char* buf) {
	int32_t be32 = 0;
	::memcpy(&be32, buf, sizeof(be32));
	return be32toh(be32);
}

MessagePtr ProtobufCodec::parse(const char* buf, int len, ErrorCode* error) {
	MessagePtr message;

	int32_t expectedCheckSum = asInt32(buf + len - kHeaderlen);
	int32_t checkSum = static_cast<int32_t>(
		::adler32(1,
			reinterpret_cast<const Bytef*>(buf),
			static_cast<int>(len - kHeaderlen)));

	if (checkSum == expectedCheckSum) {
		// get message type name
		int32_t nameLen = asInt32(buf);
		if (nameLen >= 2 && nameLen <= len - 2 * kHeaderlen) {
			std::string typeName(buf + kHeaderlen, buf + kHeaderlen + nameLen - 1);
			// create message object
			message.reset(createMessage(typeName));
			if (message) {
				// parse from buffer
				const char* data = buf + kHeaderlen + nameLen;
				int32_t dataLen = len - nameLen - 2 * kHeaderlen;
				if (message->ParseFromArray(data, dataLen)) {
					*error = kNoError;
				} else {
					*error = kParseError;
				}
			} else {
				*error = kUnknownMessageType;
			}
		} else {
			*error = kInvalidNameLength;
		}
	} else {
		*error = kChedksumError;
	}
	return message;
}

google::protobuf::Message* ProtobufCodec::createMessage(const std::string& typeName) {
	google::protobuf::Message* message = nullptr;
	const google::protobuf::Descriptor* descriptor =
			google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(typeName);
	if (descriptor) {
		const google::protobuf::Message* prototype =
			google::protobuf::MessageFactory::generated_factory()->GetPrototype(descriptor);
		if (prototype) {
			message = prototype->New();
		}
	}
	return message;
}

}	//rpc
}	//melon
