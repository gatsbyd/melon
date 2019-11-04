#include "echo.pb.h"
#include "log.h"
#include "rpc/RpcClient.h"

using namespace melon;
using namespace melon::rpc;


int main(int argc, char* argv[]) {
	if (argc < 2) {
		printf("Usage: %s ip\n", argv[0]);
	}
	IpAddress server_addr(argv[1], 5000);
	Scheduler scheduler;
	RpcClient client(server_addr, &scheduler);
	std::shared_ptr<echo::EchoRequest> request = std::make_shared<echo::EchoRequest>();
	request->set_msg("hello");

	client.Call<echo::EchoResponse>(request, [](std::shared_ptr<echo::EchoResponse> response) {
						LOG_INFO << "response: " << response->msg();
					});
	

	scheduler.start();
	return 0;
}
