#include "base/message_loop/message_pump_win.h"

#include <limits>

#include "base/memory/ptr_util.h"
#include "base/message_loop/message_loop.h"

#undef max

namespace base {

// Message sent to get an additional time slice for pumping (processing) another
// task (a series of such messages creates a continuous task pump).
static const int kMsgHaveWork = WM_USER + 1;

//-----------------------------------------------------------------------------
// MessagePumpWin public:

MessagePumpWin::MessagePumpWin() : work_state_(READY),
	                               state_(nullptr),
                                   delayed_work_time_(0)
{

}

void MessagePumpWin::Run(Delegate* delegate) 
{
    RunState s;
    s.delegate = delegate;
    s.should_quit = false;
    s.run_depth = state_ ? state_->run_depth + 1 : 1;

    RunState* previous_state = state_;
    state_ = &s;

    DoRunLoop();

    state_ = previous_state;
}

void MessagePumpWin::Quit() 
{
    state_->should_quit = true;
}

//-----------------------------------------------------------------------------
// MessagePumpWin protected:

int MessagePumpWin::GetCurrentDelay() const 
{
    if (delayed_work_time_ == 0)
        return -1;

    TimeDelta timeout = delayed_work_time_ - TimeTicksNow;

    return timeout < 0 ? 0 :
        (timeout > std::numeric_limits<int>::max() ?
        std::numeric_limits<int>::max() : static_cast<int>(timeout));
}


//-----------------------------------------------------------------------------
// MessagePumpForIO public:

MessagePumpForIO::IOContext::IOContext()
{
    memset(&overlapped, 0, sizeof(overlapped));
}

MessagePumpForIO::MessagePumpForIO() 
{
    port_.Set(CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr,
                                     reinterpret_cast<ULONG_PTR>(nullptr), 1));
}

MessagePumpForIO::~MessagePumpForIO() {

}

void MessagePumpForIO::ScheduleWork() 
{
    if (InterlockedExchange(&work_state_, HAVE_WORK) != READY)
        return;  // Someone else continued the pumping.

    // Make sure the MessagePump does some work for us.
    BOOL ret = PostQueuedCompletionStatus(port_.Get(), 0,
                                          reinterpret_cast<ULONG_PTR>(this),
                                          reinterpret_cast<OVERLAPPED*>(this));
    if (ret)
        return;  // Post worked perfectly.

    // See comment in MessagePumpForUI::ScheduleWork() for this error recovery.
    InterlockedExchange(&work_state_, READY);  // Clarify that we didn't succeed.
}

void MessagePumpForIO::ScheduleDelayedWork(const TimeTicks& delayed_work_time)
{
    // We know that we can't be blocked right now since this method can only be
    // called on the same thread as Run, so we only need to update our record of
    // how long to sleep when we do sleep.
    delayed_work_time_ = delayed_work_time;
}

void MessagePumpForIO::RegisterIOHandler(HANDLE file_handle,
                                         IOHandler* handler) 
{
    HANDLE port = CreateIoCompletionPort(file_handle, port_.Get(),
                                         reinterpret_cast<ULONG_PTR>(handler), 1);
}

bool MessagePumpForIO::RegisterJobObject(HANDLE job_handle,
    IOHandler* handler) 
{
    JOBOBJECT_ASSOCIATE_COMPLETION_PORT info;
    info.CompletionKey = handler;
    info.CompletionPort = port_.Get();
    return SetInformationJobObject(job_handle,
                                   JobObjectAssociateCompletionPortInformation,
                                   &info,
                                   sizeof(info)) != FALSE;
}

//-----------------------------------------------------------------------------
// MessagePumpForIO private:

void MessagePumpForIO::DoRunLoop() 
{
    for (;;) 
    {
        // If we do any work, we may create more messages etc., and more work may
        // possibly be waiting in another task group.  When we (for example)
        // WaitForIOCompletion(), there is a good chance there are still more
        // messages waiting.  On the other hand, when any of these methods return
        // having done no work, then it is pretty unlikely that calling them
        // again quickly will find any work to do.  Finally, if they all say they
        // had no work, then it is a good time to consider sleeping (waiting) for
        // more work.

        bool more_work_is_plausible = state_->delegate->DoWork();
        if (state_->should_quit)
            break;

        more_work_is_plausible |= WaitForIOCompletion(0, nullptr);
        if (state_->should_quit)
            break;

        more_work_is_plausible |= state_->delegate->DoDelayedWork(&delayed_work_time_);
        if (state_->should_quit)
            break;

        if (more_work_is_plausible)
            continue;

        more_work_is_plausible = state_->delegate->DoIdleWork();
        if (state_->should_quit)
            break;

        if (more_work_is_plausible)
            continue;

        WaitForWork();  // Wait (sleep) until we have work to do again.
    }
}

// Wait until IO completes, up to the time needed by the timer manager to fire
// the next set of timers.
void MessagePumpForIO::WaitForWork() 
{
    int timeout = GetCurrentDelay();
    if (timeout < 0)  // Negative value means no timers waiting.
        timeout = INFINITE;

    WaitForIOCompletion(timeout, nullptr);
}

bool MessagePumpForIO::WaitForIOCompletion(DWORD timeout, IOHandler* filter) 
{
    IOItem item;
    if (completed_io_.empty() || !MatchCompletedIOItem(filter, &item)) 
    {
        // We have to ask the system for another IO completion.
        if (!GetIOItem(timeout, &item))
            return false;

        if (ProcessInternalIOItem(item))
            return true;
    }

    if (filter && item.handler != filter) 
    {
        // Save this item for later
        completed_io_.push_back(item);
    } 
    else 
    {
        item.handler->OnIOCompleted(item.context, item.bytes_transfered, item.error);
    }
    return true;
}

// Asks the OS for another IO completion result.
bool MessagePumpForIO::GetIOItem(DWORD timeout, IOItem* item)
{
    memset(item, 0, sizeof(*item));
    ULONG_PTR key = reinterpret_cast<ULONG_PTR>(nullptr);
    OVERLAPPED* overlapped = nullptr;
    if (!GetQueuedCompletionStatus(port_.Get(), &item->bytes_transfered, &key,
        &overlapped, timeout))
    {
        if (!overlapped)
            return false;  // Nothing in the queue.
        item->error = GetLastError();
        item->bytes_transfered = 0;
    }

    item->handler = reinterpret_cast<IOHandler*>(key);
    item->context = reinterpret_cast<IOContext*>(overlapped);
    return true;
}

bool MessagePumpForIO::ProcessInternalIOItem(const IOItem& item) 
{
    if (reinterpret_cast<void*>(this) == reinterpret_cast<void*>(item.context) &&
        reinterpret_cast<void*>(this) == reinterpret_cast<void*>(item.handler)) 
    {
        // This is our internal completion.
        InterlockedExchange(&work_state_, READY);
        return true;
    }
    return false;
}

// Returns a completion item that was previously received.
bool MessagePumpForIO::MatchCompletedIOItem(IOHandler* filter, IOItem* item)
{
    for (std::list<IOItem>::iterator it = completed_io_.begin();
         it != completed_io_.end(); ++it)
    {
        if (!filter || it->handler == filter) 
        {
            *item = *it;
            completed_io_.erase(it);
            return true;
        }
    }
    return false;
}

}  // namespace base