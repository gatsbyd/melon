#include "rpc/RpcServer.h"
#include "Scheduler.h"
#include "Log.h"

#include "echo.pb.h"

using namespace melon;
using namespace melon::rpc;

MessagePtr onEcho(std::shared_ptr<echo::EchoRequest> request) {
	LOG_INFO << "server receive request, message:" << request->msg();
	std::shared_ptr<echo::EchoResponse> response(new echo::EchoResponse);
	response->set_msg(request->msg());
	return response;
}

int main() {
	Logger::setLogLevel(LogLevel::INFO);
	Singleton<Logger>::getInstance()->addAppender("console", LogAppender::ptr(new ConsoleAppender()));
	Scheduler scheduler;
	IpAddress addr("127.0.0.1", 5000);
	RpcServer server(addr, &scheduler);
	server.registerRpcHandler<echo::EchoRequest>(onEcho);

	server.start();
	scheduler.start();
	return 0;
}
