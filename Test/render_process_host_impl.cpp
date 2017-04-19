#include "render_process_host_impl.h"


namespace content {

RenderProcessHostImpl::RenderProcessHostImpl(const QString& channel_name,
    const scoped_refptr<base::SingleThreadTaskRunner>& ipc_task_runner, QObject *parent)
	: QObject(parent),
	  RenderProcessHost(channel_name.toStdString(), ipc_task_runner)
{
	child_process_launcher_ = new ChildProcessLauncher(this, this);
}

RenderProcessHostImpl::~RenderProcessHostImpl()
{

}


void RenderProcessHostImpl::OnProcessLaunched(qint64 pid)
{

}

void RenderProcessHostImpl::LaunchChildProcess(const QString& program, const QStringList& param)
{
	child_process_launcher_->Launch(program, param);
}


}


