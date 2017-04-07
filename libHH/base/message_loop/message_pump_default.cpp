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
    // ���DoWork����true����ʾ����˹�������������DoDelayedWork���������DoIdleWork
    // 
    // ���DoDelayedWork����true����ʾ����˹������������DoIdleWork

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
				//��������Ѿ���ʱ��������Ҫ����������
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