#include "test.h"
#include <QtWidgets/QApplication>
#include <stdint.h>
#include "ChildWidget.h"
#include <QWindow>
#include "base/at_exit.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	bool is_client = false;
	int32_t hwnd;
	for (int i = 0; i < argc; i++)
	{
		const QString lowerArgument = QString::fromLatin1(argv[i]).toLower();
		if (lowerArgument == QString::fromLatin1("-client"))
		{
			is_client = true;
		}
		else if (lowerArgument == QString::fromLatin1("-hwnd"))
		{
			hwnd = QString::fromLatin1(argv[++i]).toInt();
		}
	}

	base::AtExitManager exit_manager;

	if (!is_client)
	{
		Test w;
		w.show();
		return a.exec();
	}
	else
	{
		QWindow* parent = QWindow::fromWinId(hwnd);

		ChildWidget w;
		w.setWindowTitle("child");
		w.show();
		return a.exec();
	}

}
