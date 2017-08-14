#ifndef TEST_H
#define TEST_H

#include <stdint.h>
#include <QtWidgets/QMainWindow>
#include "ui_test.h"

#include "base/single_thread_task_runner.h"
#include "base/memory/ref_counted.h"

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
	Test(const scoped_refptr<base::SingleThreadTaskRunner>& ipc_task_runner, QWidget *parent = 0);
	~Test();

public slots:
	void on_pushButton_clicked();
	void on_pushButton_2_clicked();

	void OnTestCallback(int id, QString s);

    int addNUm(int num1, int num2);

private:
	Ui::TestClass ui;

	content::RenderProcessHostImpl* process_host_;

	scoped_refptr<base::SingleThreadTaskRunner> ipc_task_runner_;

	int index_;

    QList<content::RenderProcessHostImpl*> process_host_list_;
};

#endif // TEST_H
