#ifndef TEST_H
#define TEST_H

#include <stdint.h>
#include <QtWidgets/QMainWindow>
#include "ui_test.h"

namespace content{
	class RenderProcessHostImpl;
}
namespace base {
	class Thread;
}

class Test : public QMainWindow
{
	Q_OBJECT

public:
	Test(QWidget *parent = 0);
	~Test();

public slots:
	void on_pushButton_clicked();
	void on_pushButton_2_clicked();

	void OnTestCallback(int id, QString s);

private:
	Ui::TestClass ui;

	content::RenderProcessHostImpl* process_host_;

	base::Thread* thread_;

	int index_;
};

#endif // TEST_H
