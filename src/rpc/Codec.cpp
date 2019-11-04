#include "Codec.h"
#include "Buffer.h"

#include <zlib.h>

namespace melon {
namespace rpc {

void ProtobufCodec::send(const MessagePtr& message) {
	Buffer::Ptr buf(new Buffer);

	const std::string& typeName = message->GetTypeName();
	int32_t nameLen = static_cast<int32_t>(typeName.size()+1);
	buf->appendInt32(nameLen);
	buf->append(typeName.c_str(), nameLen);

	int byte_size = message->ByteSizeLong();
	buf->ensureWritableBytes(byte_size);

	uint8_t* start = reinterpret_cast<uint8_t*>(buf->beginWrite());
	message->SerializeWithCachedSizesToArray(start);
	buf->hasWritten(byte_size);

	int32_t checkSum = static_cast<int32_t>(
			::adler32(1,
					reinterpret_cast<const Bytef*>(buf->peek()),
					static_cast<int>(buf->readableBytes())));
	buf->appendInt32(checkSum);
	assert(buf->readableBytes() == sizeof nameLen + nameLen + byte_size + sizeof checkSum);
	int32_t len = htobe32(static_cast<int32_t>(buf->readableBytes()));
	buf->prepend(&len, sizeof len);

	conn_->write(buf);
}

ProtobufCodec::ErrorCode ProtobufCodec::receive(MessagePtr& message) {
	Buffer::Ptr buf(new Buffer);
	while (conn_->read(buf) > 0) {
		if (buf->readableBytes() >= kHeaderlen + kMinMessageLen) {
			const int32_t len = buf->peekInt32();
			if (len > kMaxMessageLen || len < kMinMessageLen) {
				return kInvalidLength;
			} else if (buf->readableBytes() >= static_cast<size_t>(len + kHeaderlen)) {
				ErrorCode errorcode = kNoError;
				message = parse(buf->peek() + kHeaderlen, len, &errorcode);
				return errorcode;
			}
		}
	}
	return kServerClosed;
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
