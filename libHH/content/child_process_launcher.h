#ifndef CHILD_PROCESS_LAUNCHER_H__
#define CHILD_PROCESS_LAUNCHER_H__

#include <QObject>
#include <QProcess>

#include "content/content_export.h"

class QProcess;

namespace content {

class CONTENT_EXPORT ChildProcessLauncher : public QObject
{
	Q_OBJECT

public:
	class CONTENT_EXPORT Client
	{
	public:
		virtual void OnProcessLaunched(qint64 pid) {}

		virtual void OnProcessLaunchFailed() {}

	protected:
		virtual ~Client() {}
	};


public:
	ChildProcessLauncher(Client* client, QObject *parent = 0);
	virtual ~ChildProcessLauncher();

public:
	void Launch(const QString& program, const QStringList& arguments);

private slots:
	void OnStarted();
	void OnFinished(int exitCode, QProcess::ExitStatus exitStatus);
	void OnError(QProcess::ProcessError error);

private:
	Client* client_;
	QList<QProcess*> processes_;

};


}


#endif // child_process_launcher_h__
