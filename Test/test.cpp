#include "test.h"
#include "render_process_host_impl.h"
#include <functional>

#include <QWindow>
#include <QThread>
#include <QDebug>
#include <QMessageBox>


#include "ChildWidget.h"
#include "message_define.h"
#include "logdata.h"
#include "message_traits.h"

#include "Base/lazy_instance.h"
#include "base/Message_Loop/message_pump_ui.h"
#include "base/Threading/thread_local.h"
#include "base/Threading/thread.h"
#include "base/callback.h"


base::LazyInstance<ChildWidget>::Leaky g_widget = LAZY_INSTANCE_INITIALIZER;


Test::Test(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	process_host_ = new content::RenderProcessHostImpl("testipc", this);

	layout()->setMargin(0);

	index_ = 0;

	thread_ = new base::Thread;
}

Test::~Test()
{
	thread_->Stop();
	delete thread_;
}


void Test::on_pushButton_clicked()
{
	QString text = ui.lineEdit->text();

	TestMsg_Text msg(text.toStdString());
	process_host_->Send(&msg);

	Logdata d;
	d.id = 11;
	d.name = "ss";
	d.time = 22;

	TestMsg_Log log(d);
	process_host_->Send(&log);

	Loglive d2;
	d2.id = 1112;
	d2.flows = 309;
	d2.num = 123;
	TestMsg_Live live(d2);
	process_host_->Send(&live);

	
	thread_->Start();

//	base::MessageLoop *m = new base::MessageLoop(base::MessageLoop::TYPE_UI);
//	base::MessageLoopForUI::current()->Run();

	for (int i = 1; i < 100; i++)
	{
		base::Closure cb = std::bind(&Test::OnTestCallback, this, i, "hello");
		
		thread_->message_loop()->PostDelayedTask(cb, 1000*i);

//		m->PostDelayedTask(cb, 1000*i);
	}

}

void Test::on_pushButton_2_clicked()
{
	QString path = QApplication::applicationFilePath();
	WId id = ui.tabWidget->winId();
	
	QStringList param;
	param.append("-client");
	param.append("-hwnd");
	param.append(QString::number(id));

	process_host_->LaunchChildProcess(path, param);

	g_widget.Get().OnFilterRemoved();

    char *buf = new char[10];
    std::unique_ptr<char> hh(buf);
    std::unique_ptr<char> hh2 = std::move(hh);
    char *s = hh.get();
    hh2.release();
    s = hh2.get();
    
}

void Test::OnTestCallback(int id, QString s)
{
	qDebug() << id;

	return ;
}
