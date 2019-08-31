#include "TcpServer.h"
#include "Log.h"

#include <stdio.h>
#include <unistd.h>

using namespace melon;

class Sender : public TcpServer {
public:
	Sender(const IpAddress& listen_addr, std::string filename)
		: TcpServer(listen_addr),
   		filename_(filename)	{}

	void handleClient(TcpConnection::Ptr conn) override {
		std::unique_ptr<FILE, std::function<void(FILE*)> > fp(fopen(filename_.c_str(), "rb"), [](FILE *fp) {
							fclose(fp);
						});

		if (!fp) {
			return;
		}

		sleep(5);

		char buf[8192];
		size_t nread = 0;
		while ((nread = fread(buf, 1, sizeof buf, fp.get())) > 0) {
			conn->writen(buf, nread);
		}
		
		conn->shutdown();
		while ((nread = conn->read(buf, sizeof buf) > 0)) {

		}
	}

private:
	std::string filename_;
};

int main(int argc, char* argv[]) {
	if (argc < 3) {
		printf("Usage: %s filename port\n", argv[0]);
	}

	Singleton<Logger>::getInstance()->addAppender("console", LogAppender::ptr(new ConsoleAppender()));

	IpAddress listen_addr(atoi(argv[2]));
	Sender sender(listen_addr, argv[1]);
	sender.start();

	return 0;
}
