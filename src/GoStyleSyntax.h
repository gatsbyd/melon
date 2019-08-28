#ifndef _MELON_GO_STYLE_H_
#define _MELON_GO_STYLE_H_

#include "Scheduler.h"
#include "Processer.h"

#define go ::melon::_go()-

namespace melon {

struct _go {
    void operator-(std::function<void()> task)
    {
		Processer* processer = Processer::GetProcesserOfThisThread();
		if (processer != nullptr) {
			processer->addTask(task);
		} else {
			SchedulerSingleton::getInstance()->addTask(task);
		}
    }
};

}

#endif
