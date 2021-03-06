#include "pending_task.h"
#include "callback.h"

namespace base
{

PendingTask::PendingTask(const Closure& task)
	: task(task),
	  sequence_num(0),
	  delayed_run_time(0)
{

}

PendingTask::PendingTask(const Closure& task, TimeTicks delayed_run_time)
	: task(task),
	  sequence_num(0),
	  delayed_run_time(delayed_run_time)
{
	
}

PendingTask::~PendingTask()
{

}

bool PendingTask::operator<(const PendingTask& other) const
{
	// Since the top of a priority queue is defined as the "greatest" element, we
	// need to invert the comparison here.  We want the smaller time to be at the
	// top of the heap.

	if (delayed_run_time < other.delayed_run_time)
		return false;

	if (delayed_run_time > other.delayed_run_time)
		return true;

	// If the times happen to match, then we use the sequence number to decide.
	// Compare the difference to support integer roll-over.
	return (sequence_num - other.sequence_num) > 0;
}

void TaskQueue::Swap(TaskQueue* queue) {
	c.swap(queue->c);  // Calls std::deque::swap.
}

}


