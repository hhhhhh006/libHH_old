#ifndef CHILDWIDGET_H
#define CHILDWIDGET_H

#include <QWidget>
#include "ui_ChildWidget.h"

#include "IPC/message_filter.h"
#include "logdata.h"


namespace content{
class RenderProcess;
}

class ChildWidget : public QWidget, public IPC::MessageFilter
{
	Q_OBJECT

public:
	ChildWidget(QWidget *parent = 0);
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
