#include "http/Http.h"

#include <iostream>


void testRequest() {
	melon::http::HttpRequest request;
	request.setMethod(melon::http::HttpMethod::POST);
	request.setPath("/");
	request.setQuery("color=white&size=10");
	request.setFragment("fragment1");
	request.setHeader("Connection", "keep-alive");
	request.setHeader("Accept-Language", "en-US,en;q=0.8");
	request.setContent("this is body");

	std::cout << request.toString();
}

void tesetResponse() {
	melon::http::HttpResponse response;
	response.setHttpStatus(melon::http::HttpStatus::OK);
	response.setHeader("Content-Type", "text/html;charset=utf-8");
	response.setContent("this is response body");

	response.toStream(std::cout);
}

int main() {
	testRequest();
	//tesetResponse();
	return 0;
}
