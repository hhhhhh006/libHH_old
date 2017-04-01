#include "ChildWidget.h"
#include "content/render_process.h"
#include "IPC/ipc_message_macros.h"
#include "message_define.h"

#include <stdint.h>

ChildWidget::ChildWidget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	process_ = new content::RenderProcess("testipc");
	process_->AddFilter(this);

	layout()->setMargin(0);
}

ChildWidget::~ChildWidget()
{

}


bool ChildWidget::OnMessageReceived(const IPC::Message& message)
{
	IPC_BEGIN_MESSAGE_MAP(ChildWidget, message)
	  IPC_MESSAGE_HANDLER(TestMsg_Text, OnShowText)
	  IPC_MESSAGE_HANDLER(TestMsg_Log,  OnLogdata)
	  IPC_MESSAGE_HANDLER(TestMsg_Live, OnLiveData)
	IPC_END_MESSAGE_MAP()

	return true;
}

void ChildWidget::OnShowText(std::string text)
{
	ui.textEdit->append(QString::fromUtf8(text.data()));
}

void ChildWidget::OnLogdata(Logdata data)
{
	ui.textEdit->append(QString::number(data.id));
	ui.textEdit->append(QString::fromUtf8(data.name.data()));
	ui.textEdit->append(QString::number(data.time));
}

void ChildWidget::OnLiveData(Loglive data)
{
	ui.textEdit->append(QString::number(data.id));
	ui.textEdit->append(QString::number(data.flows));
	ui.textEdit->append(QString::number(data.num));
}



