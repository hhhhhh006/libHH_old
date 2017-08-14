#ifndef MESSAGE_PUMP_H__
#define MESSAGE_PUMP_H__

#include "base/base_export.h"
#include "base/time2.h"

namespace base {

class BASE_EXPORT MessagePump
{
public:
	class BASE_EXPORT Delegate
	{
	public:
		virtual ~Delegate() {}

		virtual bool DoWork() = 0;

		virtual bool DoDelayedWork(TimeTicks* next_delayed_work_time) = 0;

		virtual bool DoIdleWork() = 0;
	};

	MessagePump();
	virtual ~MessagePump();


	virtual void Run(Delegate* delegate) = 0;

	// Quit immediately from the most recently entered run loop.  This method may
	// only be used on the thread that called Run.
	virtual void Quit() = 0;

	// Schedule a DoWork callback to happen reasonably soon.  Does nothing if a
	// DoWork callback is already scheduled.  This method may be called from any
	// thread.  Once this call is made, DoWork should not be "starved" at least
	// until it returns a value of false.
	virtual void ScheduleWork() = 0;

	// Schedule a DoDelayedWork callback to happen at the specified time,
	// cancelling any pending DoDelayedWork callback.  This method may only be
	// used on the thread that called Run.
	virtual void ScheduleDelayedWork(const TimeTicks& delayed_work_time) = 0;
};


}

#endif // message_pump_h__