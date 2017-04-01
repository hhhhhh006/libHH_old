#ifndef THREAD_H
#define THREAD_H

#include <QThread>

#include "base/base_export.h"
#include "base/message_loop/message_loop.h"

namespace base {

class BASE_EXPORT Thread : private QThread
{
//	Q_OBJECT

public:
	Thread(QObject *parent = 0);
	~Thread();

public:

	bool Start();

	void Stop();
	
	bool IsRunning() const;

	// Returns the message loop for this thread.  Use the MessageLoop's
	// PostTask methods to execute code on the thread.  This only returns
	// non-null after a successful call to Start.  After Stop has been called,
	// this will return nullptr.
	//
	// NOTE: You must not call this MessageLoop's Quit method directly.  Use
	// the Thread's Stop method instead.
	MessageLoop* message_loop() const {
		return message_loop_; 
	}

	// Returns a TaskRunner for this thread. Use the TaskRunner's PostTask
	// methods to execute code on the thread. Returns nullptr if the thread is not
	// running (e.g. before Start or after Stop have been called). Callers can
	// hold on to this even after the thread is gone; in this situation, attempts
	// to PostTask() will fail.
	SingleThreadTaskRunner* task_runner() const {
		return message_loop_ ? message_loop_->task_runner() : nullptr; 
	}

protected:
	virtual void run();

private:

	// The thread's message loop.  Valid only while the thread is alive.
	MessageLoop* message_loop_;

};


}
#endif // THREAD_H
