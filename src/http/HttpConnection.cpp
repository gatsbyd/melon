#include "http/HttpConnection.h"
#include "http/HttpParser.h"
#include "Log.h"

#include <sstream>
#include <memory>

namespace melon {
namespace http {

static const size_t s_buffer_size = 4 * 1024;

HttpConnection::HttpConnection(TcpConnection::Ptr tcp_conn)
	:tcp_conn_(tcp_conn),
	buffer_(new char[s_buffer_size], [](char* ptr) {
												delete ptr;	
											}) {
}

HttpRequest::Ptr HttpConnection::recvRequest() {
	HttpParser::Ptr parser = std::make_shared<HttpParser>();
	HttpRequest::Ptr request = std::make_shared<HttpRequest>();

	char* data = buffer_.get();
	int result = -1;
	int total_read = 0;

	do {
		int readn = tcp_conn_->read(data + total_read, s_buffer_size - total_read);
		if (readn < 0) {
			LOG_ERROR << "read < 0";
		   	return nullptr;
		} else if (readn == 0) {
			return request;
		} else {
			total_read += readn;
		}

		result = parser->parseRequest(*request, data, total_read);
		if (result > 0) {
			break;
		} else if (result == -1) {
			return nullptr;
		}

	} while(true);

	return request;
}

void HttpConnection::sendResponse(HttpResponse::Ptr response) {
	std::stringstream ss;
	response->toStream(ss);
	std::string rsp = ss.str();
	tcp_conn_->writen(rsp.c_str(), rsp.size());
}

}
}
