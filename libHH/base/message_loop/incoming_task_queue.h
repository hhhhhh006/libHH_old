#ifndef INCOMING_TASK_QUEUE_H__
#define INCOMING_TASK_QUEUE_H__

#include <stdint.h>

#include "base/base_export.h"
#include "base/callback.h"
#include "base/pending_task.h"
#include "base/time2.h"
#include "base/synchronization/lock.h"
#include "base/memory/ref_counted.h"

namespace base {
class MessageLoop;

namespace internal {

// Implements a queue of tasks posted to the message loop running on the current
// thread. This class takes care of synchronizing posting tasks from different
// threads and together with MessageLoop ensures clean shutdown.
class BASE_EXPORT IncomingTaskQueue
	: public RefCountedThreadSafe<IncomingTaskQueue>
{
public:
	explicit IncomingTaskQueue(MessageLoop* message_loop);

	bool AddToIncomingQueue(const Closure& task, TimeDelta delay);

	// Loads tasks from the |incoming_queue_| into |*work_queue|. Must be called
	// from the thread that is running the loop. Returns the number of tasks that
	// require high resolution timers.
	int ReloadWorkQueue(TaskQueue* work_queue);

	// Disconnects |this| from the parent message loop.
	void WillDestroyCurrentMessageLoop();

	// This should be called when the message loop becomes ready for
	// scheduling work.
	void StartScheduling();

private:
	friend class RefCountedThreadSafe<IncomingTaskQueue>;
	virtual ~IncomingTaskQueue();

	// Calculates the time at which a PendingTask should run.
	TimeTicks CalculateDelayedRuntime(TimeDelta delay);

	// Adds a task to |incoming_queue_|. The caller retains ownership of
	// |pending_task|, but this function will reset the value of
	// |pending_task->task|. This is needed to ensure that the posting call stack
	// does not retain |pending_task->task| beyond this function call.
	bool PostPendingTask(PendingTask* pending_task);

	// Wakes up the message loop and schedules work.
	void ScheduleWork();

	Lock incoming_queue_lock_;

	// An incoming queue of tasks that are acquired under a mutex for processing
	// on this instance's thread. These tasks have not yet been been pushed to
	// |message_loop_|.
	TaskQueue incoming_queue_;

	// Points to the message loop that owns |this|.
	MessageLoop* message_loop_;

	// The next sequence number to use for delayed tasks.
	int next_sequence_num_;

	// True if our message loop has already been scheduled and does not need to be
	// scheduled again until an empty reload occurs.
	bool message_loop_scheduled_;

	// True if we always need to call ScheduleWork when receiving a new task, even
	// if the incoming queue was not empty.
	const bool always_schedule_work_;

	// False until StartScheduling() is called.
	bool is_ready_for_scheduling_;

};


}

}
#endif // INCOMING_TASK_QUEUE_H__