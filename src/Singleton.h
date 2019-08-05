#ifndef _MELON_SINGLETON_H_
#define _MELON_SINGLETON_H_

#include <pthread.h>
#include <memory>
#include <iostream>

namespace melon {

template <typename T>
class Singleton {
public:
	static std::shared_ptr<T> getInstance() {
		pthread_once(&once_control, [&](){
							value_ = std::make_shared<T>();	
						});
		return value_;
	}
	static void destroy() {
		value_.reset();
	}

private:
	Singleton();
	~Singleton();

	static std::shared_ptr<T> value_;
	static pthread_once_t once_control;
};

template <typename T>
pthread_once_t Singleton<T>::once_control = PTHREAD_ONCE_INIT;

template <typename T>
std::shared_ptr<T> Singleton<T>::value_;

}

#endif
