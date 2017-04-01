#ifndef RENDER_PROCESS_HOST_H
#define RENDER_PROCESS_HOST_H

#include "content/content_export.h"
#include "content/process_impl.h"

namespace IPC{
	class Channel;
}

namespace content {

class CONTENT_EXPORT RenderProcessHost : public ProcessImpl
{

public:
	RenderProcessHost(const QString& channel_name);
	virtual ~RenderProcessHost();

public:
	virtual bool Send(IPC::Message* msg);
	virtual bool OnMessageReceived(const IPC::Message& message);
	virtual void OnChannelConnected(int32_t peer_pid);

protected:
	IPC::Channel* channel_;
};

}


#endif // RENDER_PROCESS_HOST_H
