#ifndef BASE_THREAD_TASK_RUNNER_HANDLE_H_
#define BASE_THREAD_TASK_RUNNER_HANDLE_H_

#include "base/base_export.h"
#include "base/memory/ref_counted.h"

namespace base {

class SingleThreadTaskRunner;

class BASE_EXPORT ThreadTaskRunnerHandle 
{
public:
    // Gets the SingleThreadTaskRunner for the current thread.
    static scoped_refptr<SingleThreadTaskRunner> Get();

    // Returns true if the SingleThreadTaskRunner is already created for
    // the current thread.
    static bool IsSet();

    // Binds |task_runner| to the current thread. |task_runner| must belong
    // to the current thread for this to succeed.
    explicit ThreadTaskRunnerHandle(
        const scoped_refptr<SingleThreadTaskRunner>& task_runner);
    ~ThreadTaskRunnerHandle();

private:
    scoped_refptr<SingleThreadTaskRunner> task_runner_;
};

}  // namespace base

#endif  // BASE_THREAD_TASK_RUNNER_HANDLE_H_
