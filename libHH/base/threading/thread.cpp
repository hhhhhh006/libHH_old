#include "thread.h"


namespace base {

Thread::Thread(ID id)
	: QThread(nullptr),
      id_(id),
	  message_loop_(nullptr)
      
{
	if (id_ == IO)
        message_loop_type_ = MessageLoop::TYPE_IO;
    else
        message_loop_type_ = MessageLoop::TYPE_DEFAULT;

}

Thread::~Thread()
{
	Stop();
}

bool Thread::Start()
{
	if (isRunning())
		return true;

	std::unique_ptr<MessageLoop> message_loop = MessageLoop::CreateUnbound(
		message_loop_type_);
	message_loop_ = message_loop.get();

	QThread::start();
	if (isRunning()) {
		message_loop.release();
		return true;
	}

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
	std::unique_ptr<MessageLoop> message_loop(message_loop_);
	message_loop_->BindToCurrentThread();
	message_loop_->Run();

	message_loop_ = nullptr;
}

}
