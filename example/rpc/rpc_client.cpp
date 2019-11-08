#include "echo.pb.h"
#include "args.pb.h"
#include "Log.h"
#include "rpc/RpcClient.h"

#include <assert.h>
#include <memory>

using namespace melon;
using namespace melon::rpc;
using namespace cherry;

std::shared_ptr<RequestAppendArgs> constructAppendArgs() {
	std::shared_ptr<RequestAppendArgs> append_args(new RequestAppendArgs);
	append_args->set_term(3);
	append_args->set_leader_id(100);
	append_args->set_pre_log_index(1);
	append_args->set_pre_log_term(1);
	append_args->set_leader_commit(1);

	LogEntry* entry = append_args->add_entries();
	entry->set_term(1);
	entry->set_index(1);
	entry->set_command("command data");
	assert(append_args->entries_size() == 1);

	return append_args;
}


int main(int argc, char* argv[]) {
	if (argc < 2) {
		printf("Usage: %s ip\n", argv[0]);
		return 0;
	}
	Logger::setLogLevel(LogLevel::INFO);
	Singleton<Logger>::getInstance()->addAppender("console", LogAppender::ptr(new ConsoleAppender()));

	IpAddress server_addr(argv[1], 5000);
	Scheduler scheduler;
	RpcClient client(server_addr, &scheduler);

	client.Call<RequestAppendReply>(constructAppendArgs(), [](std::shared_ptr<RequestAppendReply> append_reply) {
						LOG_INFO << "term=" << append_reply->term() << ", success=" << append_reply->success();
					});

	/**
	std::shared_ptr<echo::EchoRequest> request(new echo::EchoRequest);
	request->set_msg("hello");
	client.Call<echo::EchoResponse>(request, [](std::shared_ptr<echo::EchoResponse> response) {
						LOG_INFO << "client receive response, message:" << response->msg();
					});
	std::shared_ptr<echo::UnregisterRequest> unregister_request(new echo::UnregisterRequest);
	unregister_request->set_id(1);
	client.Call<echo::EchoResponse>(unregister_request, [](std::shared_ptr<echo::EchoResponse> response) {
						LOG_INFO << "client receive response, message:" << response->msg();
					});
	**/


	scheduler.start();
	return 0;
}
