#ifndef TASK_RUNNER_H__
#define TASK_RUNNER_H__

#include <stdint.h>

#include "base/base_export.h"
#include "base/callback.h"

namespace base
{

class BASE_EXPORT TaskRunner
{
public:
	// Posts the given task to be run.  Returns true if the task may be
	// run at some point in the future, and false if the task definitely
	// will not be run.
	//
	// Equivalent to PostDelayedTask(task, 0).
	bool PostTask(const Closure& task);

	// Like PostTask, but tries to run the posted task only after
	// |delay_ms| has passed.
	//
	// It is valid for an implementation to ignore |delay_ms|; that is,
	// to have PostDelayedTask behave the same as PostTask.
	virtual bool PostDelayedTask(const Closure& task, int64_t delay) = 0;

	// Returns true if the current thread is a thread on which a task
	// may be run, and false if no task will be run on the current
	// thread.
	//
	// It is valid for an implementation to always return true, or in
	// general to use 'true' as a default value.
	virtual bool RunsTasksOnCurrentThread() const = 0;

protected:
	TaskRunner();
	virtual ~TaskRunner();

};

}

#endif // TASK_RUNNER_H__