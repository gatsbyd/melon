#ifndef _MELON_COROUTINE_H_
#define _MELON_COROUTINE_H_

#include "Noncopyable.h"

#include <functional>
#include <memory>
#include <stdint.h>
#include <ucontext.h>

namespace melon {

enum class CoroutineState {
	INIT,
	RUNNABLE,
	BLOCKED,
	TERMINATED,
	EXCEPTED
};

const uint32_t kStatckSize = 1024 * 128;

class Coroutine : public Noncopyable, public std::enable_shared_from_this<Coroutine> {
public:
	typedef std::function<void ()> Func;
	typedef std::shared_ptr<Coroutine> Ptr;

	Coroutine(Func cb, uint32_t stack_size = 1024 * 128);
	Coroutine();
	~Coroutine();

	//切换到当前线程的主协程
	static void Yield();
	//执行当前协程
	void resume();
	static uint64_t GetCid();
	
private:
	static void RunInCoroutine();
	static void EnsureMainCoroutine();

	uint64_t c_id_;

	ucontext_t context_;
	Func cb_;

	uint32_t stack_size_;
	void* stack_;

	CoroutineState state_;
};

}

#endif
