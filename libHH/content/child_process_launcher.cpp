#include "child_process_launcher.h"


namespace content {

ChildProcessLauncher::ChildProcessLauncher(Client* client, QObject *parent)
	:client_(client) , QObject(parent)
{

}


ChildProcessLauncher::~ChildProcessLauncher()
{

}

void ChildProcessLauncher::Launch(const QString& program, const QStringList& arguments)
{
	QProcess* process = new QProcess(this);
	connect(process, SIGNAL(started()), this, SLOT(OnStarted()));
	connect(process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(OnFinished(int, QProcess::ExitStatus)));
	connect(process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(OnError(QProcess::ProcessError)));

	process->start(program, arguments);
	
	processes_.append(process);
}

void ChildProcessLauncher::OnStarted()
{
	QProcess* process = qobject_cast<QProcess*>(sender());
	client_->OnProcessLaunched(process->processId());
}

void ChildProcessLauncher::OnFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
	QProcess* process = qobject_cast<QProcess*>(sender());
	processes_.removeOne(process);
	delete process;
}

void ChildProcessLauncher::OnError(QProcess::ProcessError error)
{
	if (error == QProcess::FailedToStart)
	{
		client_->OnProcessLaunchFailed();
	}
}


}


