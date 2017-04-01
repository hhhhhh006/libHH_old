#ifndef WAITABLE_EVENT_H
#define WAITABLE_EVENT_H

#include <QMutex>
#include <QWaitCondition>

#include "base/base_export.h"


namespace base {

class BASE_EXPORT WaitableEvent
{
public:
	WaitableEvent();
	~WaitableEvent();

public:
	bool Wait(unsigned long time = ULONG_MAX);

	void Wake();

	void WakeOne();
	void WakeAll();

private:
	QWaitCondition wait_;
	QMutex mutex_;

};

}



#endif // WAITABLE_EVENT_H
