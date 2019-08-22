#include "http/HttpServer.h"
#include "Log.h"

int main() {
	melon::Singleton<melon::Logger>::getInstance()->addAppender("console", melon::LogAppender::ptr(new melon::ConsoleAppender()));

	melon::IpAddress addr(80);
	melon::http::HttpServer server(addr);

	server.start();
	return 0;
}
