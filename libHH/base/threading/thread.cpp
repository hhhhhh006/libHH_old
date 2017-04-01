#include "thread.h"


namespace base {

Thread::Thread(QObject *parent)
	: QThread(parent),
	  message_loop_(nullptr)
{
	
}

Thread::~Thread()
{
	Stop();
}

bool Thread::Start()
{
	if (isRunning())
		return true;

	message_loop_ = MessageLoop::CreateUnbound(MessageLoop::TYPE_DEFAULT);
	QThread::start();

	if (isRunning())
		return true;

	delete message_loop_;
	message_loop_ = nullptr;
	return false;
}

void Thread::Stop()
{
	if (isFinished() || !message_loop_)
		return;

	message_loop_->QuitWhenIdle();
	wait();
}

bool Thread::IsRunning() const
{
	return isRunning();
}

void Thread::run()
{
	message_loop_->BindToCurrentThread();
	message_loop_->Run();

	message_loop_ = nullptr;
}

}
