#include "render_process_host_impl.h"
#include "message_define.h"

namespace content {

RenderProcessHostImpl::RenderProcessHostImpl(const QString& channel_name, QObject *parent)
	: QObject(parent),
	  RenderProcessHost(channel_name)
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


