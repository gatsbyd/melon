#include "http/HttpConnection.h"
#include "http/HttpRequestParser.h"
#include "Log.h"

#include <sstream>
#include <memory>

namespace melon {
namespace http {

static const size_t s_buffer_size = 4 * 1024 * 1024;

HttpRequest::Ptr HttpConnection::recvRequest() {
	HttpRequestParser::Ptr parser = std::make_shared<HttpRequestParser>();
	HttpRequest::Ptr request = std::make_shared<HttpRequest>();
	std::unique_ptr<char, std::function<void (char*)>> buffer(
					new char[s_buffer_size], [](char* ptr) {
												delete ptr;	
											});
	char* data = buffer.get();
	HttpParserResult result = HttpParserResult::INCOMPLETED;

	do {
		int n = tcp_conn_->read(data, s_buffer_size);
		if (n < 0) {
			LOG_ERROR << "read < 0";
		   	return nullptr;
		}
		if (n == 0 && result == HttpParserResult::INCOMPLETED) {
			return nullptr;
		}
		result = parser->parse(*request, data, data + n);
		if (result == HttpParserResult::COMPLETED) {
			break;
		} else if (result == HttpParserResult::ERROR) {
			return nullptr;
		}
	} while(true);

	return request;
}

void HttpConnection::sendResponse(HttpResponse::Ptr response) {
	std::stringstream ss;
	response->toStream(ss);
	std::string rsp = ss.str();
	tcp_conn_->writeFixSize(rsp.c_str(), rsp.size());
}

}
}
