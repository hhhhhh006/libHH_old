#ifndef BASE_MESSAGE_LOOP_MESSAGE_PUMP_WIN_H_
#define BASE_MESSAGE_LOOP_MESSAGE_PUMP_WIN_H_

#include <windows.h>

#include <list>
#include <memory>

#include "base/base_export.h"
#include "base/message_loop/message_pump.h"
#include "base/time.h"
#include "base/win/scoped_handle.h"

namespace base {


class BASE_EXPORT MessagePumpWin : public MessagePump {
 public:
  MessagePumpWin();

  // MessagePump methods:
  void Run(Delegate* delegate);
  void Quit();

 protected:
  struct RunState {
    Delegate* delegate;

    // Used to flag that the current Run() invocation should return ASAP.
    bool should_quit;

    // Used to count how many Run() invocations are on the stack.
    int run_depth;
  };

  // State used with |work_state_| variable.
  enum WorkState {
    READY = 0,      // Ready to accept new work.
    HAVE_WORK = 1,  // New work has been signalled.
    WORKING = 2     // Handling the work.
  };

  virtual void DoRunLoop() = 0;
  int GetCurrentDelay() const;

  // The time at which delayed work should run.
  TimeTicks delayed_work_time_;

  // A value used to indicate if there is a kMsgDoWork message pending
  // in the Windows Message queue.  There is at most one such message, and it
  // can drive execution of tasks when a native message pump is running.
  LONG work_state_;

  // State for the current invocation of Run.
  RunState* state_;
};

//-----------------------------------------------------------------------------
// MessagePumpForIO extends MessagePumpWin with methods that are particular to a
// MessageLoop instantiated with TYPE_IO. This version of MessagePump does not
// deal with Windows mesagges, and instead has a Run loop based on Completion
// Ports so it is better suited for IO operations.
//
class BASE_EXPORT MessagePumpForIO : public MessagePumpWin 
{
public:
    struct BASE_EXPORT IOContext
    {
        IOContext();
        OVERLAPPED overlapped;
    };

    class IOHandler 
    {
    public:
        virtual ~IOHandler() {}
        // This will be called once the pending IO operation associated with
        // |context| completes. |error| is the Win32 error code of the IO operation
        // (ERROR_SUCCESS if there was no error). |bytes_transfered| will be zero
        // on error.
        virtual void OnIOCompleted(IOContext* context, DWORD bytes_transfered,
            DWORD error) = 0;
    };

    MessagePumpForIO();
    ~MessagePumpForIO();

    // MessagePump methods:
    void ScheduleWork() override;
    void ScheduleDelayedWork(const TimeTicks& delayed_work_time) override;

    // Register the handler to be used when asynchronous IO for the given file
    // completes. The registration persists as long as |file_handle| is valid, so
    // |handler| must be valid as long as there is pending IO for the given file.
    void RegisterIOHandler(HANDLE file_handle, IOHandler* handler);

    // Register the handler to be used to process job events. The registration
    // persists as long as the job object is live, so |handler| must be valid
    // until the job object is destroyed. Returns true if the registration
    // succeeded, and false otherwise.
    bool RegisterJobObject(HANDLE job_handle, IOHandler* handler);

    // Waits for the next IO completion that should be processed by |filter|, for
    // up to |timeout| milliseconds. Return true if any IO operation completed,
    // regardless of the involved handler, and false if the timeout expired. If
    // the completion port received any message and the involved IO handler
    // matches |filter|, the callback is called before returning from this code;
    // if the handler is not the one that we are looking for, the callback will
    // be postponed for another time, so reentrancy problems can be avoided.
    // External use of this method should be reserved for the rare case when the
    // caller is willing to allow pausing regular task dispatching on this thread.
    bool WaitForIOCompletion(DWORD timeout, IOHandler* filter);

private:
    struct IOItem {
        IOHandler* handler;
        IOContext* context;
        DWORD bytes_transfered;
        DWORD error;
    };

    void DoRunLoop();
    void WaitForWork();
    bool MatchCompletedIOItem(IOHandler* filter, IOItem* item);
    bool GetIOItem(DWORD timeout, IOItem* item);
    bool ProcessInternalIOItem(const IOItem& item);

    // The completion port associated with this thread.
    win::ScopedHandle port_;
    // This list will be empty almost always. It stores IO completions that have
    // not been delivered yet because somebody was doing cleanup.
    std::list<IOItem> completed_io_;
};

}  // namespace base

#endif  // BASE_MESSAGE_LOOP_MESSAGE_PUMP_WIN_H_