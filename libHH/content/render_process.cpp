#include "render_process.h"

#include "IPC/ipc_channel.h"
#include "IPC/ipc_message.h"

#include <Windows.h>


namespace content {


RenderProcess::RenderProcess(const QString& channel_name)
{
	channel_ = IPC::Channel::CreateClient(channel_name, this);
	channel_->Connect();
}

RenderProcess::~RenderProcess()
{
	delete channel_;
}


bool RenderProcess::Send(IPC::Message* msg)
{
	return channel_->Send(msg);
}

bool RenderProcess::OnMessageReceived(const IPC::Message& message)
{
	return TryFilters(message);
}

void RenderProcess::OnChannelConnected(int32_t peer_pid)
{
	int32_t pid = ::GetCurrentProcessId();

	IPC::Message msg(MSG_ROUTING_NONE, IPC::Channel::HELLO_MESSAGE_TYPE);
	msg.WriteInt32(pid);
	Send(&msg);
}


}