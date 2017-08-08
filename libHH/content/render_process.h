#ifndef RENDER_PROCESS_H
#define RENDER_PROCESS_H

#include "base/single_thread_task_runner.h"
#include "ipc/ipc_sender.h"
#include "ipc/ipc_listener.h"
#include "ipc/ipc_channel_proxy.h"
#include "content/content_export.h"


namespace content {

class CONTENT_EXPORT RenderProcess : public IPC::Sender,
								     public IPC::Listener
{

public:
	RenderProcess(const IPC::ChannelHandle& channel_handle,
        const scoped_refptr<base::SingleThreadTaskRunner>& ipc_task_runner);
	virtual ~RenderProcess();

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

#endif // RENDER_PROCESS_H
