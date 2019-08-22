#include "http/HttpParser.h"
#include "Log.h"

int main() {
	melon::Singleton<melon::Logger>::getInstance()->addAppender("console", melon::LogAppender::ptr(new melon::ConsoleAppender()));

	melon::http::HttpParser parser;
	melon::http::HttpRequest request;

	char part1[] = "POST /post_identity_body_world?q=search#hey HTTP/1.1\r\n"
 		"Accept: */*\r\n"
 		"Transfer-Encoding: identity\r\n"
 		"Content-Length: 5\r\n"
 		"\r\n"
 		"World";

	int ret = parser.parseRequest(request, part1, sizeof part1);

	LOG_DEBUG << request.getPath();
	LOG_DEBUG << request.getQuery();
	LOG_DEBUG << request.getFragment();

	if (ret > 0) {
		std::cout << request.toString();
	} else if (ret == -1) {
		LOG_DEBUG << "parser failed";
	} else if (ret == -2) {
		LOG_DEBUG << "request is patial";
	}	
	return 0;
}
