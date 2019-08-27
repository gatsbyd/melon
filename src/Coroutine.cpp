#include "Coroutine.h"
#include "Log.h"

#include <assert.h>
#include <atomic>
#include <error.h>
#include <signal.h>
#include <string.h>

namespace melon {

static thread_local uint64_t t_coroutine_id {0};

Coroutine::Coroutine(Func cb, std::string name, uint32_t stack_size)
	:c_id_(++t_coroutine_id), 
	name_(name + "-" + std::to_string(c_id_)),
	cb_(std::move(cb)),
	stack_size_(stack_size),
	state_(CoroutineState::INIT) {
	assert(stack_size > 0);
	
	LOG_DEBUG << "create coroutine:" << name_;
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
	:c_id_(++t_coroutine_id),
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
		free(stack_);
	}
	//todo:设置当前协程
}

//挂起当前正在执行的协程，切换到主协程执行，必须在非主协程调用
void Coroutine::SwapOut() {
	assert(GetCurrentCoroutine() != nullptr);

	if (GetCurrentCoroutine() == GetMainCoroutine()) {
		return;
	}

	//这里不能用智能指针，因为swapcontext切到别的协程时局部对象不会被回收，当协程执行完毕后，swapcontext之后的语句不会被执行
	Coroutine* old_coroutine = GetCurrentCoroutine().get();
	GetCurrentCoroutine() = GetMainCoroutine();

	LOG_DEBUG << "swap coroutine:" << GetCurrentCoroutine()->name()  << " in, " << "swap coroutine:" << old_coroutine->name() << " out";

	if (swapcontext(&(old_coroutine->context_), &(GetCurrentCoroutine()->context_))) {
		LOG_ERROR << "swapcontext: errno=" << errno
				<< " error string:" << strerror(errno);
	}
}

//挂起主协程，执行当前协程，只能在主协程调用
void Coroutine::swapIn() {
	if (state_ == CoroutineState::TERMINATED) {
		LOG_DEBUG << "resume a terminated coroutine " << c_id_;
		return;
	}
	Coroutine::Ptr old_coroutine = GetMainCoroutine();
	GetCurrentCoroutine() = shared_from_this();

	LOG_DEBUG << "swap coroutine:" << GetCurrentCoroutine()->name()  << " in, " << "swap coroutine:" << old_coroutine->name() << " out";

	if (swapcontext(&(old_coroutine->context_), &(GetCurrentCoroutine()->context_))) {
		LOG_ERROR << "swapcontext: errno=" << errno
				<< " error string:" << strerror(errno);
	}
}

uint64_t Coroutine::GetCid() {
	assert(GetCurrentCoroutine() != nullptr);
	return GetCurrentCoroutine()->c_id_;
}

void Coroutine::RunInCoroutine() {
	try {
		GetCurrentCoroutine()->cb_();
		GetCurrentCoroutine()->cb_ = nullptr;
	} catch (...) {
		//todo:
	}

	//重新返回主协程
	GetCurrentCoroutine()->setState(CoroutineState::TERMINATED);
	Coroutine::SwapOut();
}

Coroutine::Ptr& Coroutine::GetCurrentCoroutine() {
	//第一个协程对象调用swapIn()时初始化
	static thread_local Coroutine::Ptr t_cur_coroutine;
	return t_cur_coroutine;
}

Coroutine::Ptr Coroutine::GetMainCoroutine() {
	static thread_local Coroutine::Ptr t_main_coroutine = Coroutine::Ptr(new Coroutine());
	return t_main_coroutine;
}

std::string Coroutine::name() {
	return name_;
}

}
