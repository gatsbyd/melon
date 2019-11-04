#include "rpc/RpcServer.h"
#include "Scheduler.h"
#include "log.h"

#include "echo.pb.h"

using namespace melon;
using namespace melon::rpc;

MessagePtr onEcho(std::shared_ptr<echo::EchoRequest> request) {
	LOG_INFO << "receive request, message:" << request->msg();
	std::shared_ptr<echo::EchoResponse> response = std::shared_ptr<echo::EchoResponse>();
	response->set_msg(request->msg());
	return response;
}

int main() {
	Scheduler scheduler;
	IpAddress addr("127.0.0.1", 5000);
	RpcServer server(addr, &scheduler);
	server.registerRpcHandler<echo::EchoRequest>(onEcho);

	server.start();
	scheduler.start();
	return 0;
}
