#include "http/Http.h"

#include <iostream>
#include <sstream>

namespace melon {
namespace http {

std::string HttpMethodToString(const HttpMethod& method) {
	switch(method) { 
#define XX(method, name, desc) \
			case HttpMethod::name: return #desc;
		HTTP_METHOD_MAP(XX)
#undef XX
			default: return "UNKNOWN";
	}	
}

//todo: slow
HttpMethod StringToHttpMethod(const std::string& str) {
#define XX(method, name, desc) \
		if (#name == str) return HttpMethod::name;
		HTTP_METHOD_MAP(XX)
#undef XX
	return HttpMethod::UNKNOWN;
}

void HttpRequest::setMethod(std::string method) { 
	setMethod(StringToHttpMethod(method));
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
		<< (query_.empty() ? "" : "?")
		<< query_
		<< (fragment_.empty() ? "" : "#")
		<< fragment_ << " "
		<< "HTTP/" << major_version_ << "." << minor_version_
		<< "\r\n";
	//header
	for (auto& header : headers_) {
		os << header.first << ": " << header.second << "\r\n";
	} 
	//body
	os << "\r\n" << content_;

	return os;
}

std::string HttpRequest::toString() {
	std::stringstream ss;
	toStream(ss);
	return ss.str();
}
const std::string HttpResponse::getHeader(const std::string& key, const std::string def) {
	auto it = headers_.find(key);
	return it == headers_.end() ? def : it->second;
}

void HttpResponse::setHeader(const std::string& key, const std::string& value) {
	headers_[key] = value;
}

void HttpResponse::delHeader(const std::string& key) {
	headers_.equal_range(key);
}

int HttpStatusToCode(const HttpStatus& status) {
	return static_cast<int>(status);
}

std::string HttpStatusDesc(const HttpStatus& status) {
	switch(status) {
#define XX(code, name, desc) \
			case HttpStatus::name: return #desc;
			HTTP_STATUS_MAP(XX)
#undef XX
			default: return "UNKNOWN";
	}
}

std::ostream& HttpResponse::toStream(std::ostream& os) {
	//inital line
	os << "HTTP/" << major_version_ << "." << minor_version_ << " "
	 << HttpStatusToCode(status_) << " "
	 << HttpStatusDesc(status_) << "\r\n";
	//header
	for (auto& header : headers_) {
		os << header.first << ": " << header.second << "\r\n";
	} 
	//body
	os << "\r\n" << content_;

	return os;
}

std::string HttpResponse::toString() {
	std::stringstream ss;
	toStream(ss);
	return ss.str();
}

}
}
