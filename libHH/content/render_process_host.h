#ifndef RENDER_PROCESS_HOST_H
#define RENDER_PROCESS_HOST_H

#include "base/single_thread_task_runner.h"
#include "ipc/ipc_sender.h"
#include "ipc/ipc_listener.h"
#include "ipc/ipc_channel_proxy.h"
#include "content/content_export.h"


namespace content {

class CONTENT_EXPORT RenderProcessHost : public IPC::Sender,
								         public IPC::Listener
{
public:
	RenderProcessHost(const IPC::ChannelHandle& channel_handle, 
        const scoped_refptr<base::SingleThreadTaskRunner>& ipc_task_runner);
	virtual ~RenderProcessHost();

public:
	virtual bool Send(IPC::Message* msg);
	virtual bool OnMessageReceived(const IPC::Message& message);
	virtual void OnChannelConnected(int32_t peer_pid);

    virtual void AddFilter(IPC::MessageFilter* filter);
    virtual void RemoveFilter(IPC::MessageFilter* filter);

protected:
	std::unique_ptr<IPC::ChannelProxy> channel_;
};

}


#endif // RENDER_PROCESS_HOST_H
