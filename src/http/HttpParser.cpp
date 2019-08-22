#include "http/HttpParser.h"
#include "http/picohttpparser.h"

namespace melon {
namespace http {

static const int kMaxHeadersNum = 30;
	
int HttpParser::parseRequest(HttpRequest& request, const char* buf, size_t len) {
	const char *method;
	size_t method_len;
	const char *path;
	size_t path_len;
	int minor_version;
	struct phr_header headers[kMaxHeadersNum];
	size_t num_headers = kMaxHeadersNum;

	int ret = phr_parse_request(buf, len, 
				&method, &method_len, 
				&path, &path_len, 
				&minor_version, 
				headers, &num_headers, 0);

	if (ret > 0) {
		request.setMethod(std::string(method, method_len));
		std::string path_query_fragment(path, path_len);
		std::size_t query_start = path_query_fragment.find('?');
		std::size_t fragment_start = path_query_fragment.find('#');
		//path
		if (query_start != std::string::npos) {
			request.setPath(path_query_fragment.substr(0, query_start));
		} else if (fragment_start != std::string::npos) {
			request.setPath(path_query_fragment.substr(0, fragment_start));
		} else {
			request.setPath(path_query_fragment);
		}
		//query
		if (query_start != std::string::npos) {
			request.setQuery(path_query_fragment.substr(query_start, fragment_start));
		}
		//fragment
		if (fragment_start != std::string::npos) {
			request.setFragment(path_query_fragment.substr(fragment_start));
		}
		//minor_version
		request.setMinorVersion(minor_version);
		//headers
		for (const struct phr_header header : headers) {
			request.setHeader(std::string(header.name, header.name_len), std::string(header.value, header.value_len));
		}
	} 
	return ret;
}

}
}
