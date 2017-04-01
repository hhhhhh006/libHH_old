#ifndef MESSAGE_PUMP_DEFAULT_H
#define MESSAGE_PUMP_DEFAULT_H

#include "base/base_export.h"
#include "base/synchronization/waitable_event.h"
#include "base/message_loop/message_pump.h"

namespace base {

class MessagePumpDefault : public MessagePump
{
public:
	MessagePumpDefault();
	~MessagePumpDefault();

	// MessagePump methods:
	void Run(Delegate* delegate) override;
	void Quit() override;
	void ScheduleWork() override;
	void ScheduleDelayedWork(const TimeTicks& delayed_work_time) override;

private:
	// This flag is set to false when Run should return.
	bool keep_running_;

	// Used to sleep until there is more work to do.
	WaitableEvent event_;

	// The time at which we should call DoDelayedWork.
	TimeTicks delayed_work_time_;
	
};


}
#endif // MESSAGE_PUMP_DEFAULT_H
