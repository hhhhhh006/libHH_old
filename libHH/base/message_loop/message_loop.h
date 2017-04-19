#ifndef MESSAGE_LOOP_H__
#define MESSAGE_LOOP_H__

#include <memory>

#include "base/base_export.h"
#include "base/pending_task.h"
#include "base/memory/ref_counted.h"
#include "base/threading/thread_task_runner_handle.h"
#include "base/message_loop/message_pump.h"
#include "base/message_loop/incoming_task_queue.h"
#include "base/message_loop/message_loop_task_runner.h"
#include "base/message_loop/message_pump_win.h"

namespace base
{
struct PendingTask;
class SingleThreadTaskRunner;


class BASE_EXPORT MessageLoop : public MessagePump::Delegate
{
public:
	enum Type 
	{
		TYPE_DEFAULT,
		TYPE_UI,
		TYPE_IO,
	};

	explicit MessageLoop(Type type = TYPE_DEFAULT);

	virtual ~MessageLoop();

	// Returns the MessageLoop object for the current thread, or null if none.
	static MessageLoop* current();

	static std::unique_ptr<MessagePump> CreateMessagePumpForType(Type type);

	// NOTE: These methods may be called on any thread.  The Task will be invoked
	// on the thread that executes MessageLoop::Run().
	void PostTask(const Closure& task);

	void PostDelayedTask(const Closure& task, TimeDelta delay);

	void Run();

	// Process all pending tasks, windows messages, etc., but don't wait/sleep.
	// Return as soon as all items that can be run are taken care of.
	void RunUntilIdle();

	// Signals the Run method to return when it becomes idle. It will continue to
	// process pending messages and future messages as long as they are enqueued.
	// Warning: if the MessageLoop remains busy, it may never quit. Only use this
	// Quit method when looping procedures (such as web pages) have been shut
	// down.
	//
	// This method may only be called on the same thread that called Run, and Run
	// must still be on the call stack.
	void QuitWhenIdle();

	// This method is a variant of Quit, that does not wait for pending messages
	// to be processed before returning from Run.
	void QuitNow();

	// Deprecated: use RunLoop instead.
	// Construct a Closure that will call QuitWhenIdle(). Useful to schedule an
	// arbitrary MessageLoop to QuitWhenIdle.
	static Closure QuitWhenIdleClosure();

	// Returns true if this loop is |type|. This allows subclasses (especially
	// those in tests) to specialize how they are identified.
	virtual bool IsType(Type type) const;

	// Returns the type passed to the constructor.
	Type type() const { return type_; }

	// Gets the TaskRunner associated with this message loop.
	const scoped_refptr<SingleThreadTaskRunner>& task_runner() const { 
		return task_runner_; 
	}

	// Can only be called from the thread that owns the MessageLoop.
	bool is_running() const;

	// Runs the specified PendingTask.
	void RunTask(const PendingTask& pending_task);

protected:
	std::unique_ptr<MessagePump> pump_;

private:
	friend class internal::IncomingTaskQueue;
	friend class Thread;

	MessageLoop(Type type, int shadow);

	// Creates a MessageLoop without binding to a thread.
	// If |type| is TYPE_CUSTOM non-null |pump_factory| must be also given
	// to create a message pump for this message loop.  Otherwise a default
	// message pump for the |type| is created.
	//
	// It is valid to call this to create a new message loop on one thread,
	// and then pass it to the thread where the message loop actually runs.
	// The message loop's BindToCurrentThread() method must be called on the
	// thread the message loop runs on, before calling Run().
	// Before BindToCurrentThread() is called, only Post*Task() functions can
	// be called on the message loop.
	static std::unique_ptr<MessageLoop> CreateUnbound(Type type);

    void Init();

	// Configure various members and bind this message loop to the current thread.
	void BindToCurrentThread();

    // Sets the ThreadTaskRunnerHandle for the current thread to point to the
    // task runner for this message loop.
    void SetThreadTaskRunnerHandle();

	// Calls RunTask or queues the pending_task on the deferred task list if it
	// cannot be run right now.  Returns true if the task was run.
	bool DeferOrRunPendingTask(const PendingTask& pending_task);

	// Adds the pending task to delayed_work_queue_.
	void AddToDelayedWorkQueue(const PendingTask& pending_task);

	// Delete tasks that haven't run yet without running them.  Used in the
	// destructor to make sure all the task's destructors get called.  Returns
	// true if some work was done.
	bool DeletePendingTasks();

	// Loads tasks from the incoming queue to |work_queue_| if the latter is
	// empty.
	void ReloadWorkQueue();

	// Wakes up the message pump. Can be called on any thread. The caller is
	// responsible for synchronizing ScheduleWork() calls.
	void ScheduleWork();

	// MessagePump::Delegate methods:
	virtual bool DoWork();
	virtual bool DoDelayedWork(TimeTicks* next_delayed_work_time);
	virtual bool DoIdleWork();


	const Type type_;

	bool is_running_;

	// A recursion block that prevents accidentally running additional tasks when
	// insider a (accidentally induced?) nested message pump.
	bool nestable_tasks_allowed_;

	// Used to record that QuitWhenIdle() was called on the MessageLoop, meaning
	// that we should quit Run once it becomes idle.
	bool quit_when_idle_received_;

	// A recent snapshot of Time::Now(), used to check delayed_work_queue_.
	TimeTicks recent_time_;

	// A list of tasks that need to be processed by this instance.  Note that
	// this queue is only accessed (push/pop) by our current thread.
	TaskQueue work_queue_;

	// Contains delayed tasks, sorted by their 'delayed_run_time' property.
	DelayedTaskQueue delayed_work_queue_;

	scoped_refptr<internal::IncomingTaskQueue> incoming_task_queue_;

	// A task runner which we haven't bound to a thread yet.
	scoped_refptr<internal::MessageLoopTaskRunner> unbound_task_runner_;

	// The task runner associated with this message loop.
	scoped_refptr<SingleThreadTaskRunner> task_runner_;

   
    std::unique_ptr<ThreadTaskRunnerHandle> thread_task_runner_handle_;


	DISALLOW_COPY_AND_ASSIGN(MessageLoop);
};


class BASE_EXPORT MessageLoopForUI : public MessageLoop
{
public:
	MessageLoopForUI() : MessageLoop(TYPE_UI) {}

	static MessageLoopForUI *current() {
		MessageLoop *loop = MessageLoop::current();
		return static_cast<MessageLoopForUI*>(loop);
	}

	static bool IsCurrent() {
		MessageLoop *loop = MessageLoop::current();
		return loop && loop->type() == MessageLoop::TYPE_UI;
	}

};

//-----------------------------------------------------------------------------
// MessageLoopForIO extends MessageLoop with methods that are particular to a
// MessageLoop instantiated with TYPE_IO.
//
// This class is typically used like so:
//   MessageLoopForIO::current()->...call some method...
//
class BASE_EXPORT MessageLoopForIO : public MessageLoop 
{
public:
    MessageLoopForIO() : MessageLoop(TYPE_IO) {}

    // Returns the MessageLoopForIO of the current thread.
    static MessageLoopForIO* current() {
        MessageLoop* loop = MessageLoop::current();
        return static_cast<MessageLoopForIO*>(loop);
    }

    static bool IsCurrent() {
        MessageLoop* loop = MessageLoop::current();
        return loop && loop->type() == MessageLoop::TYPE_IO;
    }

    typedef MessagePumpForIO::IOHandler IOHandler;
    typedef MessagePumpForIO::IOContext IOContext;

    // Please see MessagePumpWin for definitions of these methods.
    void RegisterIOHandler(HANDLE file, IOHandler* handler);
    bool RegisterJobObject(HANDLE job, IOHandler* handler);
    bool WaitForIOCompletion(DWORD timeout, IOHandler* filter);

};


}

#endif // MESSAGE_LOOP_H__