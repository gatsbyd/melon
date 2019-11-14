#include "rpc/RpcServer.h"
#include "Scheduler.h"
#include "Log.h"

#include "echo.pb.h"
#include "args.pb.h"

#include <stdio.h>

using namespace melon;
using namespace melon::rpc;
using namespace cherry;

MessagePtr onEcho(std::shared_ptr<echo::EchoRequest> request) {
	LOG_INFO << "server receive request, message:" << request->msg();
	std::shared_ptr<echo::EchoResponse> response(new echo::EchoResponse);
	response->set_msg(request->msg());
	return response;
}


MessagePtr onAppendEntry(std::shared_ptr<RequestAppendArgs> append_args) {
	LOG_INFO << "new append rpc come: term=" << append_args->term() << ", leader_id=" << append_args->leader_id()
			<< ", pre_log_index=" << append_args->pre_log_index() << ", pre_log_term=" << append_args->pre_log_term()
			<< ", leader_commit=" << append_args->leader_commit();

	int log_size = append_args->entries_size();
	for (int i = 0; i < log_size; ++i) {
		const LogEntry& log_entry = append_args->entries(i);
		KvCommnad cmd;
		cmd.ParseFromString(log_entry.command());
		LOG_INFO << "log entry: term=" << log_entry.term() << ", index=" << log_entry.index()
				<< ", cmd.operation=" << cmd.operation() << ", cmd.key=" << cmd.key()
				<< ", cmd.value=" << cmd.value() << ", cmd.cid=" << cmd.cid() << ", cmd.seq=" << cmd.seq();
	}

	std::shared_ptr<RequestAppendReply> append_reply(new RequestAppendReply);
	append_reply->set_term(2);
	append_reply->set_success(true);
	return append_reply;
}


int main() {
	Logger::setLogLevel(LogLevel::INFO);
	Singleton<Logger>::getInstance()->addAppender("console", LogAppender::ptr(new ConsoleAppender()));
	Scheduler scheduler;
	scheduler.startAsync();
	IpAddress addr(5000);
	RpcServer server(addr, &scheduler);

	server.registerRpcHandler<echo::EchoRequest>(onEcho);
	server.registerRpcHandler<RequestAppendArgs>(onAppendEntry);

	server.start();
	getchar();
	return 0;
}
