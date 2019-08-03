#ifndef _MELON_NON_COPYABLE_H_
#define _MELON_NON_COPYABLE_H_

namespace melon {

class Noncopyable {
public:
	Noncopyable(const Noncopyable& rhs) = delete;
	Noncopyable& operator=(const Noncopyable& rhs) = delete;

protected:
	Noncopyable() = default;
	~Noncopyable() = default;

};

}

#endif
