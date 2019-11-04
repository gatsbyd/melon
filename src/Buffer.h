#ifndef _MELON_BUFFER_H_
#define _MELON_BUFFER_H_

#include <assert.h>
#include <algorithm>
#include <endian.h>
#include <stdio.h>
#include <string.h>
#include <vector>

#include "Socket.h"

namespace melon {

// refer to muduo by Shuo Chen
class Buffer {
public:
	typedef std::shared_ptr<Buffer> Ptr;
	static const size_t kCheapPrepend = 8;
	static const size_t kInitialSize = 1024;
	explicit Buffer(size_t initialSize = kInitialSize)
		:buffer_(kCheapPrepend + initialSize),
		read_index_(kCheapPrepend),
		write_index_(kCheapPrepend) {
		assert(readableBytes() == 0);
		assert(writableBytes() == initialSize);
		assert(prependableBytes() == kCheapPrepend);
	}
	size_t readableBytes() const {
		return write_index_ - read_index_;
	}
	size_t writableBytes() const {
		return buffer_.size() - write_index_;
	}
	size_t prependableBytes() const {
		return read_index_;
	}
	//retrieve
	void retrieve(size_t len) {
		assert(len <= readableBytes());
		if (len < readableBytes()) {
		read_index_ += len;
		} else {
			retrieveAll();
    	}
	}
	void retrieveInt8() {
		retrieve(sizeof(int8_t));
	}
	void retrieveInt16() {
		retrieve(sizeof(int16_t));
	}
	void retrieveInt32() {
		retrieve(sizeof(int32_t));
	}
	void retrieveInt64() {
		retrieve(sizeof(int64_t));
	}
	void retrieveAll() {
		read_index_ = kCheapPrepend;
		write_index_ = kCheapPrepend;
	}
	void retrieveUntil(const char* end) {
		assert(peek() <= end);
		retrieve(end - peek());
	}
	//peek
	const char* peek() const {
		return begin() + read_index_;
	}
	int8_t peekInt8() const {
		assert(readableBytes() > sizeof(int8_t));
		int8_t x = *peek();
		return x;
	}
	int16_t peekInt16() const {
		assert(readableBytes() > sizeof(int16_t));
		int16_t x = 0;
		::memcpy(&x, peek(), sizeof x);
		return be16toh(x);
	}
	int32_t peekInt32() const {
		assert(readableBytes() > sizeof(int32_t));
		int32_t x = 0;
		::memcpy(&x, peek(), sizeof x);
		return be32toh(x);
	}
	int64_t peekInt64() const {
		assert(readableBytes() > sizeof(int64_t));
		int64_t x = 0;
		::memcpy(&x, peek(), sizeof x);
		return be64toh(x);
	}
	std::string peekAsString() const {
		std::string result(peek(), readableBytes());
		return result;
	}
	//read
	int8_t readInt8() {
		int8_t result = peekInt8();
		retrieveInt8();
		return  result;
	}

	int16_t readInt16() {
		int16_t result = peekInt16();
		retrieveInt16();
		return  result;
	}
	int32_t readInt32() {
		int32_t result = peekInt32();
		retrieveInt32();
		return  result;
	}
	int64_t readInt64() {
		int64_t result = peekInt64();
		retrieveInt64();
		return  result;
	}
	std::string readAsString() {
		std::string result(peek(), readableBytes());
		retrieveAll();
		return result;
	}
	//append
	void append(const char* data, size_t len) {
		ensureWritableBytes(len);
		std::copy(data, data+len, beginWrite());
		hasWritten(len);
	}
	void append(const void* data, size_t len) {
		append(static_cast<const char*>(data), len);
	}
	void appendInt8(int8_t x) {
		append(&x, sizeof x);
	}
	void appendInt16(int16_t x) {
		x = htobe16(x);
		append(&x, sizeof x);
	}
	void appendInt32(int32_t x) {
		x = htobe32(x);
		append(&x, sizeof x);
	}
	void appendInt64(int64_t x) {
		x = htobe64(x);
		append(&x, sizeof x);
	}
	//preppand
	void prepend(const void* data, size_t len) {
		assert(len <= prependableBytes());
		read_index_ -= len;
		const char* d = static_cast<const char*>(data);
		std::copy(d, d + len, begin() + read_index_);
	}
	const char* findCRLF() const {
		const char* crlf = std::search(peek(), beginWrite(), kCRLF, kCRLF + 2);
		return crlf == beginWrite() ? nullptr : crlf;
	}
	ssize_t readSocket(Socket::Ptr socket);

	void ensureWritableBytes(size_t len) {
		if (writableBytes() < len) {
			makeSpace(len);
		}
		assert(writableBytes() >= len);
	}
	char* beginWrite() {
		return begin() + write_index_;	
	}
	const char* beginWrite() const {
		return begin() + write_index_;	
	}
	void hasWritten(size_t len) {
		assert(len <= writableBytes());
		write_index_ += len;
	}
private:
	char* begin() {
		return &*buffer_.begin();
	}
	const char* begin() const {
		return &*buffer_.begin();
	}
	void makeSpace(size_t len) {
		if (writableBytes() + prependableBytes() < len + kCheapPrepend) {
			buffer_.resize(write_index_ + len);
		} else {
			assert(kCheapPrepend < read_index_);
			size_t readable = readableBytes();
			std::copy(begin() + read_index_,
					begin() + write_index_,
					begin() + kCheapPrepend);
			read_index_ = kCheapPrepend;
			write_index_ = read_index_ + readable;
			assert(readable == readableBytes());
    }
  }

	std::vector<char> buffer_;
	size_t read_index_;
	size_t write_index_;

	static const char kCRLF[];
};

}

#endif
