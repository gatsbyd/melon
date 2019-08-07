#include "Coroutine.h"
#include "Log.h"

#include <assert.h>
#include <atomic>
#include <error.h>
#include <signal.h>
#include <string.h>

namespace melon {

static std::atomic<uint64_t> s_coroutine_id {0};
static thread_local Coroutine::Ptr s_cur_coroutine;
static thread_local Coroutine::Ptr s_main_coroutine;

void test() {}

Coroutine::Coroutine(Func cb, uint32_t stack_size)
	:c_id_(++s_coroutine_id), 
	cb_(std::move(cb)),
	stack_size_(stack_size),
	state_(CoroutineState::INIT) {
	assert(stack_size > 0);
	
	//todo:统一分配函数
	stack_ = malloc(stack_size_);
	if (!stack_) {
		LOG_ERROR << "run out of memory";
	}
	
	if (getcontext(&context_)) {
		LOG_ERROR << "getcontext: errno=" << errno
				<< " error string:" << strerror(errno);
	}
	context_.uc_link = nullptr;
	context_.uc_stack.ss_size = stack_size_;
	context_.uc_stack.ss_sp = stack_;

	makecontext(&context_, &Coroutine::RunInCoroutine, 0);
}

Coroutine::Coroutine()
	:c_id_(++s_coroutine_id),
	state_(CoroutineState::INIT) {
	
	if (getcontext(&context_)) {
		LOG_ERROR << "getcontext: errno=" << errno
				<< " error string:" << strerror(errno);
	}
}

Coroutine::~Coroutine() {
	if (stack_) {
		//todo:
		free(stack_);
	}
	//todo:设置当前协程
}

//挂起当前正在执行的协程，切换到主协程执行，必须在非主协程调用
void Coroutine::Yield() {
	if (s_cur_coroutine == s_main_coroutine) {
		return;
	}

	Coroutine::Ptr old_coroutine = s_cur_coroutine;
	s_cur_coroutine = s_main_coroutine;
	if (swapcontext(&(old_coroutine->context_), &(s_cur_coroutine->context_))) {
		LOG_ERROR << "swapcontext: errno=" << errno
				<< " error string:" << strerror(errno);
	}
}

//挂起主协程，执行当前协程，只能在主协程调用
void Coroutine::resume() {
	EnsureMainCoroutine();
	assert(s_cur_coroutine == s_main_coroutine);
	;
	s_cur_coroutine = shared_from_this();
	if (swapcontext(&(s_main_coroutine->context_), &(s_cur_coroutine->context_))) {
		LOG_ERROR << "swapcontext: errno=" << errno
				<< " error string:" << strerror(errno);
	}
}

uint64_t Coroutine::GetCid() {
	EnsureMainCoroutine();
	return s_cur_coroutine->c_id_;
}

void Coroutine::RunInCoroutine() {
	try {
		s_cur_coroutine->cb_();
		s_cur_coroutine->cb_ = nullptr;
		s_cur_coroutine->state_ = CoroutineState::TERMINATED;
	} catch (...) {
		//todo:
	}

	//重新返回主协程
	Coroutine::Yield();
}

void Coroutine::EnsureMainCoroutine() {
	if (s_main_coroutine == nullptr) {
		s_main_coroutine = std::make_shared<Coroutine>();
		s_cur_coroutine = s_main_coroutine;
	}
}

}
