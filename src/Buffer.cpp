#include "Buffer.h"
#include "Socket.h"

namespace melon {

const char Buffer::kCRLF[] = "\r\n";

ssize_t Buffer::readSocket(Socket::Ptr socket) {
	char extrabuf[10240]; //10k
	struct iovec vec[2];
	const size_t writable = writableBytes();
	vec[0].iov_base = beginWrite();
	vec[0].iov_len = writable;
	vec[1].iov_base = extrabuf;
	vec[1].iov_len = sizeof extrabuf;
	const ssize_t n = socket->readv(vec, 2);
	if (static_cast<size_t>(n) <= writable) {
		hasWritten(n);
	} else {
		write_index_ = buffer_.size();
		append(extrabuf, n - writable);
	}
	return n;
}

}
