#include "render_process_host.h"

#include "ipc/ipc_channel.h"
#include "ipc/ipc_message.h"

namespace content {


RenderProcessHost::RenderProcessHost(const IPC::ChannelHandle& channel_handle,
    const scoped_refptr<base::SingleThreadTaskRunner>& ipc_task_runner)
{
	channel_ = IPC::ChannelProxy::Create(channel_handle, IPC::Channel::MODE_SERVER, this, ipc_task_runner);
}

RenderProcessHost::~RenderProcessHost()
{
	
}


bool RenderProcessHost::Send(IPC::Message* msg)
{
	return channel_->Send(msg);
}

bool RenderProcessHost::OnMessageReceived(const IPC::Message& message)
{
	return true;
}

void RenderProcessHost::OnChannelConnected(int32_t peer_pid)
{

}

void RenderProcessHost::AddFilter(IPC::MessageFilter* filter)
{
    channel_->AddFilter(filter);
}

void RenderProcessHost::RemoveFilter(IPC::MessageFilter* filter)
{
    channel_->RemoveFilter(filter);
}

}