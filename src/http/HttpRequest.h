#ifndef _MELON_HTTP_REQUEST_H_
#define _MELON_HTTP_REQUEST_H_

#include <map>

namespace melon {

namespace http {

enum class HttpMethod {
	DELETE,
	GET,
	HEAD,
	POST,
	PUT,
	CONNECT,
	OPTION,
	TRACE
};

class HttpRequest {
public:
	//get
	HttpMethod getMethod() const { return method_; }
	const std::string& getPath() const { return path_; }
	const std::string& getQuery() const { return query_; }
	const std::string& getFragment() const { return fragment_; }
	int getMajorVersion() const { return major_version_; }
	int getMinorVersion() const { return minor_version_; }
	const std::map<std::string, std::string>& getHeaders() const { return headers_; }
	const std::string getHeader(const std::string& key, const std::string def);
	const std::string& getCotent() const { return content_; }

	//set
	void setMethod(HttpMethod method) { method_ = method; }
	void setPath(const std::string& path) { path_ = path; }
	void setQuery(const std::string& query) { query_ = query; }
	void setFragment(const std::string& fragment) { fragment_ = fragment; }
	void setMajorVersion(int major_version) { major_version_ = major_version; }
	void setMinorVersion(int minor_version) { minor_version_ = minor_version; }
	void setHeaders(const std::map<std::string, std::string>& headers) { headers_ = headers; }
	void setHeader(const std::string& key, const std::string& value);
	void setContent(const std::string& content) { content_ = content; }

	//delete
	void delHeader(const std::string& key);

	std::ostream& toStream(std::ostream& stream);

private:
	HttpMethod method_;
	std::string path_;
	std::string query_;
	std::string fragment_;
	int major_version_;
	int minor_version_;
	std::map<std::string, std::string> headers_;
	std::string content_;
};

}

}

#endif
