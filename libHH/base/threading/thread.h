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

    // ����Start�ɹ�,MessageLoop��Ч������Stop��MessageLoop������
    // ����ֱ�ӵ���MessageLoop��Quit������Ӧ�õ����̵߳�Stop����
    // ��Ӧ�ó���MessageLoop����
    // Ӧ�ó��о������ü�����SingleThreadTaskRunner����
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
