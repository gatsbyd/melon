#include "Coroutine.h"
#include "Log.h"

#include <assert.h>
#include <atomic>
#include <error.h>
#include <signal.h>
#include <string.h>

namespace melon {

static thread_local std::atomic<uint64_t> s_coroutine_id {0};
static thread_local Coroutine::Ptr s_cur_coroutine;
static thread_local Coroutine::Ptr s_main_coroutine;

Coroutine::Coroutine(Func cb, std::string name, uint32_t stack_size)
	:c_id_(++s_coroutine_id), 
	name_(name + "_" + std::to_string(c_id_)),
	cb_(std::move(cb)),
	stack_size_(stack_size),
	state_(CoroutineState::INIT) {
	assert(stack_size > 0);
	
	LOG_DEBUG << "create coroutine:" << name_;
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
	name_("Main_" + std::to_string(c_id_)),
	state_(CoroutineState::INIT) {
	LOG_DEBUG << "create coroutine:" << name_;
	
	if (getcontext(&context_)) {
		LOG_ERROR << "getcontext: errno=" << errno
				<< " error string:" << strerror(errno);
	}
}

Coroutine::~Coroutine() {
	LOG_DEBUG << "destroy coroutine:" << name_;
	if (stack_) {
		//todo:
		free(stack_);
	}
	//todo:设置当前协程
}

//挂起当前正在执行的协程，切换到主协程执行，必须在非主协程调用
void Coroutine::Yield() {
	assert(s_cur_coroutine != nullptr);
	if (s_cur_coroutine == s_main_coroutine) {
		return;
	}

	//这里不能用智能指针，因为swapcontext切到别的协程时局部对象不会被回收，当协程执行完毕后，swapcontext之后的语句不会被执行
	Coroutine* old_coroutine = s_cur_coroutine.get();
	s_cur_coroutine = s_main_coroutine;

	LOG_DEBUG << "swap coroutine:" << s_cur_coroutine->name()  << " in, " << "swap coroutine:" << old_coroutine->name() << " out";

	if (swapcontext(&(old_coroutine->context_), &(s_cur_coroutine->context_))) {
		LOG_ERROR << "swapcontext: errno=" << errno
				<< " error string:" << strerror(errno);
	}
}

//挂起主协程，执行当前协程，只能在主协程调用
void Coroutine::resume() {
	EnsureMainCoroutine();
	assert(s_cur_coroutine == s_main_coroutine);

	if (state_ == CoroutineState::TERMINATED) {
		LOG_DEBUG << "resume a terminated coroutine " << c_id_;
		return;
	}
	Coroutine::Ptr old_coroutine = s_cur_coroutine;
	s_cur_coroutine = shared_from_this();

	LOG_DEBUG << "swap coroutine:" << s_cur_coroutine->name()  << " in, " << "swap coroutine:" << old_coroutine->name() << " out";

	if (swapcontext(&(old_coroutine->context_), &(s_cur_coroutine->context_))) {
		LOG_ERROR << "swapcontext: errno=" << errno
				<< " error string:" << strerror(errno);
	}
}

uint64_t Coroutine::GetCid() {
	assert(s_cur_coroutine != nullptr);
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

Coroutine::Ptr Coroutine::GetCurrentCoroutine() {
	assert(s_cur_coroutine != nullptr);
	return s_cur_coroutine;
}

std::string Coroutine::name() {
	return name_;
}

}
