#ifndef MESSAGE_LOOP_TASK_RUNNER_H__
#define MESSAGE_LOOP_TASK_RUNNER_H__

#include <Qt>

#include "base/base_export.h"
#include "base/synchronization/lock.h"
#include "base/single_thread_task_runner.h"
#include "base/time.h"
#include "base/memory/ref_counted.h"

namespace base {
namespace internal {

class IncomingTaskQueue;

// A stock implementation of SingleThreadTaskRunner that is created and managed
// by a MessageLoop. For now a MessageLoopTaskRunner can only be created as
// part of a MessageLoop.
class BASE_EXPORT MessageLoopTaskRunner : public SingleThreadTaskRunner
{
public:
	explicit MessageLoopTaskRunner(scoped_refptr<IncomingTaskQueue> incoming_queue);

	// Initialize this message loop task runner on the current thread.
	void BindToCurrentThread();

	// SingleThreadTaskRunner implementation
	bool PostDelayedTask(const base::Closure& task, base::TimeDelta delay) override;

	bool RunsTasksOnCurrentThread() const override;

private:
	~MessageLoopTaskRunner();

	scoped_refptr<IncomingTaskQueue> incoming_queue_;
	Qt::HANDLE valid_thread_id_;
	mutable Lock valid_thread_id_lock_;
};


}

}


#endif // MESSAGE_LOOP_TASK_RUNNER_H__
