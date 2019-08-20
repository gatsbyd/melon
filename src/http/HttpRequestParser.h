#ifndef _MELON_HTTP_REQUEST_PARSER_H_
#define _MELON_HTTP_REQUEST_PARSER_H_

#include "http/Http.h"

namespace melon {
namespace http {

enum class HttpParserResult {
	COMPLETED,
	INCOMPLETED,
	ERROR
};

class HttpRequestParser {
public:
	typedef std::shared_ptr<HttpRequestParser> Ptr;
	HttpRequestParser();
	
	HttpParserResult parse(HttpRequest& request, char* begin, char* end);

private:
	enum class Status {
		
	};

	Status status_;
};

}
}

#endif
