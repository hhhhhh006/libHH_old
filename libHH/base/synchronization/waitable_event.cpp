#include "waitable_event.h"

namespace base {


WaitableEvent::WaitableEvent()
{

}

WaitableEvent::~WaitableEvent()
{

}


bool WaitableEvent::Wait(unsigned long time /*= ULONG_MAX*/)
{
	mutex_.lock();
	bool rt = wait_.wait(&mutex_, time);
	mutex_.unlock();
	return rt;
}

void WaitableEvent::Wake()
{
	WakeOne();
}

void WaitableEvent::WakeOne()
{
	wait_.wakeOne();
}

void WaitableEvent::WakeAll()
{
	wait_.wakeAll();
}

}