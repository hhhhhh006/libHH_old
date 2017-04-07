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
    enum ID {
        UI,
        IO,
        ID_COUNT
    };

	Thread(ID id);
	~Thread();

public:

	bool Start();

	void Stop();
	
	bool IsRunning() const;

    // 调用Start成功,MessageLoop有效，调用Stop，MessageLoop被销毁
    // 不能直接调用MessageLoop的Quit方法，应该调用线程的Stop方法
    // 不应该持有MessageLoop对象，
    // 应该持有具有引用计数的SingleThreadTaskRunner对象。
	MessageLoop* message_loop() const {
		return message_loop_; 
	}

	scoped_refptr<SingleThreadTaskRunner> task_runner() const {
		return message_loop_ ? message_loop_->task_runner() : nullptr; 
	}

protected:
	virtual void run();

private:

	// The thread's message loop.  Valid only while the thread is alive.
	MessageLoop* message_loop_;

    ID id_;

    MessageLoop::Type message_loop_type_;
};


}
#endif // THREAD_H
