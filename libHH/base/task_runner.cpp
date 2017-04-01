#include "task_runner.h"

namespace base
{

TaskRunner::TaskRunner(void)
{
}


TaskRunner::~TaskRunner(void)
{
}

bool TaskRunner::PostTask(const Closure& task)
{
	return PostDelayedTask(task, 0);
}

}
