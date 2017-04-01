#include "message_pump_ui.h"

#include <QCoreApplication>

namespace base {


MessagePumpQt::MessagePumpQt()
	: kMsgHaveWork_(QEvent::registerEventType(QEvent::User + 1)),
	  delayed_work_time_(0),
	  should_quit_(false),
	  delegate_(nullptr)
{
	
}

MessagePumpQt::~MessagePumpQt()
{

}

int MessagePumpQt::GetCurrentDelay() const
{
	if (delayed_work_time_ == 0)
		return -1;

	TimeDelta timeout = delayed_work_time_ - TimeTicksNow;

	return timeout < 0 ? 0 :
		(timeout > std::numeric_limits<int>::max() ?
		std::numeric_limits<int>::max() : static_cast<int>(timeout));
}

MessagePumpForUI::MessagePumpForUI()
	: timer_id_(0)
{

}

MessagePumpForUI::~MessagePumpForUI()
{

}


void MessagePumpForUI::Run(Delegate* delegate)
{
	delegate_ = delegate;

// 	for (;;)
// 	{
// 		QCoreApplication::processEvents();
// 
// 		bool did_work = delegate->DoWork();
// 		if (should_quit_)
// 			break;
// 
// 		did_work |= delegate->DoDelayedWork(&delayed_work_time_);
// 		if (did_work && delayed_work_time_ == 0)
// 			killTimer(timer_id_);
// 		if (should_quit_)
// 			break;
// 
// 		if (did_work)
// 			continue;
// 
// 		did_work = delegate->DoIdleWork();
// 		if (should_quit_)
// 			break;
// 
// 		if (did_work)
// 			continue;
// 	}

}

void MessagePumpForUI::Quit()
{

}

void MessagePumpForUI::ScheduleWork()
{
	QEvent *event = new QEvent(static_cast<QEvent::Type>(kMsgHaveWork_));
	QCoreApplication::postEvent(this, event);
}

void MessagePumpForUI::ScheduleDelayedWork(const TimeTicks& delayed_work_time)
{
	delayed_work_time_ = delayed_work_time;
	
	if (timer_id_ == 0)
		timer_id_ = startTimer(GetCurrentDelay());
}

void MessagePumpForUI::HandleWorkMessage()
{
	if (!delegate_)
		return;

	if (delegate_->DoWork())
		ScheduleWork();

	delegate_->DoDelayedWork(&delayed_work_time_);
	RescheduleTimer();
}

void MessagePumpForUI::HandleTimerMessage()
{
	killTimer(timer_id_);
	timer_id_ = 0;

	if (!delegate_)
		return;

	delegate_->DoDelayedWork(&delayed_work_time_);

	RescheduleTimer();
}

void MessagePumpForUI::RescheduleTimer()
{
	if (delayed_work_time_ == 0)
		return;

	//delay为0，代表最早延迟任务已经延迟，我们需要立即执行它
	int delay = GetCurrentDelay();
	if (delay == 0)
	{
		ScheduleWork();
	}
	else
	{
		if (timer_id_ > 0)
		{
			killTimer(timer_id_);
			timer_id_ = 0;
		}

		timer_id_ = startTimer(delay);
	}
}

bool MessagePumpForUI::event(QEvent *e)
{
	if (e->type() == kMsgHaveWork_)
	{
		HandleWorkMessage();
	}
	else if (e->type() == QEvent::Timer)
	{
		HandleTimerMessage();
	}

	return false;
}


}