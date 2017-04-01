#include "message_loop_task_runner.h"

#include <QThread>

#include "base/message_loop/incoming_task_queue.h"

namespace base {
namespace internal {


MessageLoopTaskRunner::MessageLoopTaskRunner(IncomingTaskQueue* incoming_queue)
	: incoming_queue_(incoming_queue),
	  valid_thread_id_(0)
{

}

void MessageLoopTaskRunner::BindToCurrentThread()
{
	AutoLock locker(valid_thread_id_lock_);
	valid_thread_id_ = QThread::currentThreadId();
}

bool MessageLoopTaskRunner::PostDelayedTask(const base::Closure& task, base::TimeDelta delay)
{
	return incoming_queue_->AddToIncomingQueue(task, delay);
}

bool MessageLoopTaskRunner::RunsTasksOnCurrentThread() const
{
	AutoLock locker(valid_thread_id_lock_);
	return valid_thread_id_ == QThread::currentThreadId();
}

MessageLoopTaskRunner::~MessageLoopTaskRunner()
{

}

}

}
