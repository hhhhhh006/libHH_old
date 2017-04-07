#ifndef CHILDWIDGET_H
#define CHILDWIDGET_H

#include <QWidget>
#include "ui_ChildWidget.h"

#include "ipc/message_filter.h"
#include "logdata.h"

#include "base/single_thread_task_runner.h"
#include "base/memory/ref_counted.h"

namespace content{
class RenderProcess;
}

class ChildWidget : public QWidget, public IPC::MessageFilter
{
	Q_OBJECT

public:
	ChildWidget(const scoped_refptr<base::SingleThreadTaskRunner>& ipc_task_runner, 
        const QString &pipe_name, QWidget *parent = 0);
	~ChildWidget();

    virtual bool OnMessageReceived(const IPC::Message& message);

	void OnShowText(std::string text);
	void OnLogdata(Logdata data);
	void OnLiveData(Loglive data);


private:
	Ui::ChildWidget ui;

	content::RenderProcess* process_;
};

#endif // CHILDWIDGET_H
