#include "message_pump_default.h"

namespace base {

MessagePumpDefault::MessagePumpDefault()
	: keep_running_(true)
{

}

MessagePumpDefault::~MessagePumpDefault()
{

}


void MessagePumpDefault::Run(Delegate* delegate)
{
    // 如果DoWork返回true，表示完成了工作，继续调用DoDelayedWork，不会调用DoIdleWork
    // 
    // 如果DoDelayedWork返回true，表示完成了工作，不会调用DoIdleWork

	for (;;)
	{
		bool did_work = delegate->DoWork();
		if (!keep_running_)
			break;

		did_work |= delegate->DoDelayedWork(&delayed_work_time_);
		if (!keep_running_)
			break;

		if (did_work)
			continue;

		did_work = delegate->DoIdleWork();
		if (!keep_running_)
			break;

		if (did_work)
			continue;

		if (delayed_work_time_ == 0)
		{
			event_.Wait();
		}
		else
		{
			TimeDelta delay = delayed_work_time_ - TimeTicksNow;
			if (delay > 0)
				event_.Wait(delay);
			else
				//如果任务已经超时，我们需要立即运行它
				delayed_work_time_ = 0;
		}
	}

	keep_running_ = true;
}

void MessagePumpDefault::Quit()
{
	keep_running_ = false;
}

void MessagePumpDefault::ScheduleWork()
{
	event_.Wake();
}

void MessagePumpDefault::ScheduleDelayedWork(const TimeTicks& delayed_work_time)
{
	delayed_work_time_ = delayed_work_time;
}

}