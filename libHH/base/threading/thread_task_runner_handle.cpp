#include "base/threading/thread_task_runner_handle.h"

#include "base/lazy_instance.h"
#include "base/single_thread_task_runner.h"
#include "base/threading/thread_local.h"

namespace base {

namespace {

base::LazyInstance<base::ThreadLocalPointer<ThreadTaskRunnerHandle> >::Leaky
    lazy_tls_ptr = LAZY_INSTANCE_INITIALIZER;

}  // namespace

// static
scoped_refptr<SingleThreadTaskRunner> ThreadTaskRunnerHandle::Get() 
{
    ThreadTaskRunnerHandle* current = lazy_tls_ptr.Pointer()->Get();
    return current->task_runner_;
}

// static
bool ThreadTaskRunnerHandle::IsSet() 
{
    return lazy_tls_ptr.Pointer()->Get() != NULL;
}

ThreadTaskRunnerHandle::ThreadTaskRunnerHandle(
    const scoped_refptr<SingleThreadTaskRunner>& task_runner)
    : task_runner_(task_runner) 
{
    lazy_tls_ptr.Pointer()->Set(this);
}

ThreadTaskRunnerHandle::~ThreadTaskRunnerHandle()
{
    lazy_tls_ptr.Pointer()->Set(NULL);
}

}  // namespace base
