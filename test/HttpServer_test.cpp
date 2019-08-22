#include "http/HttpServer.h"


int main() {
	melon::IpAddress addr(80);
	melon::http::HttpServer server(addr);

	server.start();
	return 0;
}
