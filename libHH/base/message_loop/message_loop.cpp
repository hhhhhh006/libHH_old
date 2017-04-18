#include "message_loop.h"

#include "base/lazy_instance.h"
#include "base/memory/ptr_util.h"
#include "base/threading/thread_local.h"
#include "base/message_loop/incoming_task_queue.h"
#include "base/message_loop/message_pump_default.h"
#include "base/message_loop/message_pump_ui.h"

namespace base {

namespace {

// A lazily created thread local storage for quick access to a thread's message
// loop, if one exists.  This should be safe and free of static constructors.
LazyInstance<base::ThreadLocalPointer<MessageLoop> >::Leaky lazy_tls_ptr = 
    LAZY_INSTANCE_INITIALIZER;

MessagePumpForIO* ToPumpIO(MessagePump* pump) {
    return static_cast<MessagePumpForIO*>(pump);
}

}


MessageLoop::MessageLoop(Type type /*= TYPE_DEFAULT*/)
	: type_(type),
	  quit_when_idle_received_(false),
	  pump_(nullptr),
	  is_running_(false),
	  recent_time_(0),
	  nestable_tasks_allowed_(true)
{
	Init();
	BindToCurrentThread();
}

void MessageLoop::Init()
{
    incoming_task_queue_ = new internal::IncomingTaskQueue(this);
    unbound_task_runner_ = new internal::MessageLoopTaskRunner(incoming_task_queue_);
    task_runner_ = unbound_task_runner_;
}

MessageLoop::~MessageLoop()
{
	bool did_work;
	for (int i = 0; i < 100; ++i) 
	{
		DeletePendingTasks();
		ReloadWorkQueue();
		// If we end up with empty queues, then break out of the loop.
		did_work = DeletePendingTasks();
		if (!did_work)
			break;
	}

	incoming_task_queue_->WillDestroyCurrentMessageLoop();
	incoming_task_queue_ = nullptr;
	unbound_task_runner_ = nullptr;
	task_runner_ = nullptr;

	lazy_tls_ptr.Pointer()->Set(nullptr);
}


MessageLoop* MessageLoop::current()
{
	return lazy_tls_ptr.Pointer()->Get();
}

std::unique_ptr<MessagePump> MessageLoop::CreateMessagePumpForType(Type type)
{
	if (type == TYPE_UI)
		return base::WrapUnique(new MessagePumpForUI());
    else if (type == TYPE_IO)
        return base::WrapUnique(new MessagePumpForIO());

	return base::WrapUnique(new MessagePumpDefault());;
}

void MessageLoop::PostTask(const Closure& task)
{
	task_runner_->PostTask(task);
}

void MessageLoop::PostDelayedTask(const Closure& task, TimeDelta delay)
{
	task_runner_->PostDelayedTask(task, delay);
}

void MessageLoop::Run()
{
	is_running_ = true;
	pump_->Run(this);
	is_running_ = false;
}

void MessageLoop::RunUntilIdle()
{
	quit_when_idle_received_ = true;
	Run();
}

void MessageLoop::QuitWhenIdle()
{
	quit_when_idle_received_ = true;
}

void MessageLoop::QuitNow()
{
	if (is_running_ && pump_)
	{
		pump_->Quit();
	}
}

static void QuitCurrentWhenIdle() {
	MessageLoop::current()->QuitWhenIdle();
}

base::Closure MessageLoop::QuitWhenIdleClosure()
{
	base::Closure closure = std::bind(&QuitCurrentWhenIdle);
	return std::bind(&QuitCurrentWhenIdle);
}

bool MessageLoop::IsType(Type type) const
{
	return type_ == type;
}

bool MessageLoop::is_running() const
{
	return is_running_;
}

void MessageLoop::RunTask(const PendingTask& pending_task)
{
	nestable_tasks_allowed_ = false;

	pending_task.task.Run();

	nestable_tasks_allowed_ = true;
}

void MessageLoop::BindToCurrentThread()
{
	pump_ = CreateMessagePumpForType(type_);

	lazy_tls_ptr.Pointer()->Set(this);

	incoming_task_queue_->StartScheduling();
	unbound_task_runner_->BindToCurrentThread();
	unbound_task_runner_ = nullptr;

    SetThreadTaskRunnerHandle();
}


void MessageLoop::SetThreadTaskRunnerHandle()
{
    thread_task_runner_handle_ = new ThreadTaskRunnerHandle(task_runner_);
}

std::unique_ptr<MessageLoop> MessageLoop::CreateUnbound(Type type)
{
	return std::unique_ptr<MessageLoop>(new MessageLoop(type, 1));
}


MessageLoop::MessageLoop(Type type, int shadow)
	: type_(type),
	  quit_when_idle_received_(false),
	  pump_(nullptr),
	  is_running_(false),
	  recent_time_(0),
	  nestable_tasks_allowed_(true)
{
	Init();
}


bool MessageLoop::DeferOrRunPendingTask(const PendingTask& pending_task)
{
	RunTask(pending_task);
	return true;
}

void MessageLoop::AddToDelayedWorkQueue(const PendingTask& pending_task)
{
	delayed_work_queue_.push(pending_task);
}

bool MessageLoop::DeletePendingTasks()
{
	bool did_work = !work_queue_.empty();
	while (!work_queue_.empty())
	{
		PendingTask pending_task = work_queue_.front();
		work_queue_.pop();
		if (!pending_task.delayed_run_time != 0)
		{
			AddToDelayedWorkQueue(pending_task);
		}
	}
  
	did_work |= !delayed_work_queue_.empty();

	while (!delayed_work_queue_.empty())
		delayed_work_queue_.pop();

	return did_work;
}

void MessageLoop::ReloadWorkQueue()
{
    if (work_queue_.empty())
	{
		incoming_task_queue_->ReloadWorkQueue(&work_queue_);
	}
}

void MessageLoop::ScheduleWork()
{
	pump_->ScheduleWork();
}

bool MessageLoop::DoWork()
{
	if (!nestable_tasks_allowed_)
		return false;

	for (;;)
	{
		ReloadWorkQueue();
		if (work_queue_.empty())
			break;

		// Execute oldest task.
		do 
		{
			PendingTask pending_task = work_queue_.front();
			work_queue_.pop();
			if (pending_task.delayed_run_time != 0)
			{
				AddToDelayedWorkQueue(pending_task);
				pump_->ScheduleDelayedWork(delayed_work_queue_.top().delayed_run_time);
			} 
			else 
			{
				if (DeferOrRunPendingTask(pending_task))
					return true;
			}
		} while (!work_queue_.empty());
	}

	// Nothing happened.
	return false;
}

bool MessageLoop::DoDelayedWork(TimeTicks* next_delayed_work_time)
{
	if (!nestable_tasks_allowed_ || delayed_work_queue_.empty())
	{
		recent_time_ = *next_delayed_work_time = 0;
		return false;
	}

	// When we "fall behind", there will be a lot of tasks in the delayed work
	// queue that are ready to run.  To increase efficiency when we fall behind,
	// we will only call Time::Now() intermittently, and then process all tasks
	// that are ready to run before calling it again.  As a result, the more we
	// fall behind (and have a lot of ready-to-run delayed tasks), the more
	// efficient we'll be at handling the tasks.

	TimeTicks next_run_time = delayed_work_queue_.top().delayed_run_time;
	if (next_run_time > recent_time_)
	{
		recent_time_ = TimeTicksNow;
		if (next_run_time > recent_time_)
		{
			*next_delayed_work_time = next_run_time;
			return false;
		}
	}

	PendingTask pending_task = delayed_work_queue_.top();
	delayed_work_queue_.pop();

	if (!delayed_work_queue_.empty())
		*next_delayed_work_time = delayed_work_queue_.top().delayed_run_time;

	return DeferOrRunPendingTask(pending_task);
}

bool MessageLoop::DoIdleWork()
{
	if (quit_when_idle_received_)
		pump_->Quit();

	return false;
}


//MessageLoopForIO methods
void MessageLoopForIO::RegisterIOHandler(HANDLE file, IOHandler* handler)
{
    return ToPumpIO(pump_.get())->RegisterIOHandler(file, handler);
}

bool MessageLoopForIO::RegisterJobObject(HANDLE job, IOHandler* handler)
{
    return ToPumpIO(pump_.get())->RegisterJobObject(job, handler);
}

bool MessageLoopForIO::WaitForIOCompletion(DWORD timeout, IOHandler* filter)
{
    return ToPumpIO(pump_.get())->WaitForIOCompletion(timeout, filter);
}


}


