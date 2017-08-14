#include "test.h"

#include <QWindow>
#include <QThread>
#include <QDebug>
#include <QMessageBox>


#include "ChildWidget.h"
#include "logdata.h"
#include "message_define.h"

#include "base/logging.h"
#include "base/callback.h"
#include "base/lazy_instance.h"
#include "base/Message_Loop/message_pump_ui.h"
#include "base/Threading/thread_local.h"
#include "base/Threading/thread.h"
#include "render_process_host_impl.h"

base::ThreadLocalBoolean g_local_bool_;

Test::Test(const scoped_refptr<base::SingleThreadTaskRunner>& ipc_task_runner, QWidget *parent)
	: QMainWindow(parent),
      ipc_task_runner_(ipc_task_runner)
{
	ui.setupUi(this);

	layout()->setMargin(0);

	index_ = 0; 
}

Test::~Test()
{
    for (int i = 0; i < process_host_list_.size(); i++)
    {
        delete process_host_list_[i];
    }
}


void Test::on_pushButton_clicked()
{
	QString text = ui.lineEdit->text();

    Logdata d;
    d.id = 1111111;
    d.name = "ssssssss";
    d.time = 1111111;

    Loglive data;
    data.id = 222222;
    data.flows = 22222;
    data.num = 2222222;
    data.name = "dddddddd";
    data.time = 2222222;

    for (int i = 0; i < process_host_list_.size(); i++)
    {
        process_host_list_[i]->Send(new TestMsg_Text(text.toStdString()));
        process_host_list_[i]->Send(new TestMsg_Log(d));
        process_host_list_[i]->Send(new TestMsg_Live(data));
    }


	for (int i = 1; i < 100; i++)
	{
	//	base::Closure cb = std::bind(&Test::OnTestCallback, this, i, "hello");

	//	io_thread_->message_loop()->PostDelayedTask(cb, 1000*i);

	//	base::MessageLoopForUI::current()->PostDelayedTask(cb, 1000*i);
	}
}

void Test::on_pushButton_2_clicked()
{
	QString path = QApplication::applicationFilePath();
	WId id = ui.tabWidget->winId();
	
    index_++;
    QString pipe_name = QString("ipc") + QString::number(index_);

	QStringList param;
	param.append("-client");
	param.append("-pipe");
	param.append(pipe_name);
  
    process_host_ = new content::RenderProcessHostImpl(pipe_name, ipc_task_runner_);
    process_host_list_.append(process_host_);
	process_host_->LaunchChildProcess(path, param);
}

void Test::OnTestCallback(int id, QString s)
{
	qDebug() << id << s;

	return;
}

int Test::addNUm(int num1, int num2)
{
    return num1 + num2;
}
