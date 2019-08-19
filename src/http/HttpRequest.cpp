#include "http/HttpRequest.h"

#include <iostream>

namespace melon {
namespace http {

std::string HttpMethodToString(HttpMethod method) {
	std::string name;
	switch (method) {
		case HttpMethod::DELETE:
			 name = "DELETE";
			 break;
		case HttpMethod::GET:
			 name = "GET";
			 break;
		case HttpMethod::HEAD:
			 name = "HEAD";
			 break;
		case HttpMethod::POST:
			 name = "POST";
			 break;
		case HttpMethod::PUT:
			 name = "PUT";
			 break;
		case HttpMethod::CONNECT:
			 name = "CONNECT";
			 break;
		case HttpMethod::OPTION:
			 name = "OPTION";
			 break;
		case HttpMethod::TRACE:
			 name = "TRACE";
			 break;
		default:
			 name = "UNKNOWN";
			
	}
	return name;
}

const std::string HttpRequest::getHeader(const std::string& key, const std::string def) {
	auto it = headers_.find(key);
	return it == headers_.end() ? def : it->second;
}

void HttpRequest::setHeader(const std::string& key, const std::string& value) {
	headers_[key] = value;
}

void HttpRequest::delHeader(const std::string& key) {
	headers_.equal_range(key);
}

std::ostream& HttpRequest::toStream(std::ostream& os) {
	//inital line
	os << HttpMethodToString(method_) << " "
		<< path_ 
		<< (query_.empty()	? "" : "?")
		<< query_
		<< (fragment_.empty() ? "" : "#") << " "
		<< "HTTP/" << major_version_ << "." << minor_version_
		<< "\r\n";
	//header
	for (auto& header : headers_) {
		os << header.first << ": " << header.second << "\r\n";
	} 
	if (!content_.empty()) {
		os << "content-length: " << content_.size() << "\r\n";
	}
	//body
	os << "\r\n" << content_;

	return os;
}

}
}
