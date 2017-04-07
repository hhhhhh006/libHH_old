#ifndef SINGLE_THREAD_TASK_RUNNER_H__
#define SINGLE_THREAD_TASK_RUNNER_H__

#include "base/base_export.h"
#include "base/task_runner.h"

namespace base {

class BASE_EXPORT SingleThreadTaskRunner : public TaskRunner
{
public:
	// A more explicit alias to RunsTasksOnCurrentThread().
	bool BelongsToCurrentThread() const{
		return RunsTasksOnCurrentThread();
	}

protected:
	~SingleThreadTaskRunner() {}
};

}

#endif // SINGLE_THREAD_TASK_RUNNER_H__
