#ifndef _MELON_HTTP_PARSER_H_
#define _MELON_HTTP_PARSER_H_

#include "http/Http.h"

#include <memory>

namespace melon {
namespace http {


class HttpParser {
public:
	typedef std::shared_ptr<HttpParser> Ptr;
	HttpParser() = default;

	static int parseRequest(HttpRequest& request, const char* buf, size_t len);
};

}
}

#endif
