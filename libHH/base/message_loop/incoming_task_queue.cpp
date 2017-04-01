#include "incoming_task_queue.h"

#include "message_loop.h"

namespace base {
namespace internal {


IncomingTaskQueue::IncomingTaskQueue(MessageLoop* message_loop)
	: message_loop_(message_loop),
	  next_sequence_num_(0),
	  message_loop_scheduled_(false),
	  always_schedule_work_(false),
	  is_ready_for_scheduling_(false)
{

}

bool IncomingTaskQueue::AddToIncomingQueue(const Closure& task, TimeDelta delay)
{
	AutoLock locker(incoming_queue_lock_);
	PendingTask pending_task(task, CalculateDelayedRuntime(delay));
	
	return PostPendingTask(&pending_task);
}

int IncomingTaskQueue::ReloadWorkQueue(TaskQueue* work_queue)
{
	AutoLock locker(incoming_queue_lock_);
	if (incoming_queue_.empty())
	{
		message_loop_scheduled_ = false;
	}
	else
	{
		incoming_queue_.Swap(work_queue);
	}
	return work_queue->size();
}

void IncomingTaskQueue::WillDestroyCurrentMessageLoop()
{
	AutoLock locker(incoming_queue_lock_);
	message_loop_ = nullptr;
}

void IncomingTaskQueue::StartScheduling()
{
	AutoLock locker(incoming_queue_lock_);
	is_ready_for_scheduling_ = true;
	if (!incoming_queue_.empty())
		ScheduleWork();
}

IncomingTaskQueue::~IncomingTaskQueue()
{

}

TimeTicks IncomingTaskQueue::CalculateDelayedRuntime(TimeDelta delay)
{
	TimeTicks delayed_run_time = 0;
	if (delay > 0)
		delayed_run_time = QDateTime::currentMSecsSinceEpoch() + delay;

	return delayed_run_time;
}

bool IncomingTaskQueue::PostPendingTask(PendingTask* pending_task)
{
	if (!message_loop_)
	{
		return false;
	}

	pending_task->sequence_num = next_sequence_num_++;

	bool was_empty = incoming_queue_.empty();
	incoming_queue_.push(*pending_task);

	if (is_ready_for_scheduling_ && (always_schedule_work_ || (!message_loop_scheduled_ && was_empty)))
	{
		ScheduleWork();
	}
	return true;
}

void IncomingTaskQueue::ScheduleWork()
{
	if (message_loop_)
	{
		message_loop_->ScheduleWork();
		message_loop_scheduled_ = true;
	}
}


}

}