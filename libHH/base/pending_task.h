#ifndef PENDING_TASK_H__
#define PENDING_TASK_H__

#include <stdint.h>
#include <queue>

#include "base/base_export.h"
#include "base/callback.h"
#include "base/time.h"

namespace base {

struct BASE_EXPORT PendingTask
{
	PendingTask(const Closure& task);
	
	PendingTask(const Closure& task, TimeTicks delayed_run_time);

	~PendingTask();

	bool operator<(const PendingTask& other) const;

	// The task to run.
	base::Closure task;

	// Secondary sort key for run time.
	int sequence_num;

	// The time when the task should be run.
	TimeTicks delayed_run_time;
};

// Wrapper around std::queue specialized for PendingTask which adds a Swap
// helper method.
class BASE_EXPORT TaskQueue : public std::queue<PendingTask> 
{
public:
	void Swap(TaskQueue* queue);
};

// PendingTasks are sorted by their |delayed_run_time| property.
typedef std::priority_queue<base::PendingTask> DelayedTaskQueue;


}


#endif // PENDING_TASK_H__
