#include "render_process_host.h"

#include "IPC/ipc_channel.h"
#include "IPC/ipc_message.h"

namespace content {


RenderProcessHost::RenderProcessHost(const QString& channel_name)
{
	channel_ = IPC::Channel::CreateServer(channel_name, this);
}

RenderProcessHost::~RenderProcessHost()
{
	delete channel_;
}


bool RenderProcessHost::Send(IPC::Message* msg)
{
	return channel_->Send(msg);
}

bool RenderProcessHost::OnMessageReceived(const IPC::Message& message)
{
	return TryFilters(message);
}

void RenderProcessHost::OnChannelConnected(int32_t peer_pid)
{

}

}