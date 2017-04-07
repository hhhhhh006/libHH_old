#include "test.h"
#include <QtWidgets/QApplication>
#include <stdint.h>

#include "ChildWidget.h"

#include "base/lazy_instance.h"
#include "base/message_loop/message_loop.h"
#include "base/threading/thread.h"


int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	bool is_client = false;
	QString pipe_name;
	for (int i = 0; i < argc; i++)
	{
		const QString lowerArgument = QString::fromLatin1(argv[i]).toLower();
		if (lowerArgument == QString::fromLatin1("-client"))
		{
			is_client = true;
		}
		else if (lowerArgument == QString::fromLatin1("-pipe"))
		{
			pipe_name = QString::fromLatin1(argv[++i]);
		}
	}

    // 需要先创建一个ui消息循环
    // 用于 io 线程的listen接口代理回 ui线程
    base::MessageLoop *loop = new base::MessageLoop(base::MessageLoopForUI::TYPE_UI);
    base::MessageLoopForUI::current()->Run();

    // ipc通信在io线程处理
    base::Thread *io_thread = new base::Thread(base::Thread::IO);
    io_thread->Start();

	if (!is_client)
	{
		Test w(io_thread->task_runner());
		w.show();
		return a.exec();
	}
	else
	{
		ChildWidget w(io_thread->task_runner(), pipe_name);
		w.setWindowTitle("child");
		w.show();
		return a.exec();
	}

}
