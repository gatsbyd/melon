#ifndef _MELON_THREAD_H_
#define _MELON_THREAD_H_

#include <functional>

namespace melon {

class Thread {
public:
	typedef std::function<void (void)> Func;
	Thread(Func cb, std::string name = "");
	~Thread();

	void start();
	void join();
	
	static pid_t CurrentThreadTid();
private:
	static void* threadFuncInternal(void* arg);
	bool started_;
	bool joined_;
	pthread_t tid_;
	std::string name_;
	Func cb_;
};

}

#endif 
