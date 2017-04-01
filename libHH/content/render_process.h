#ifndef RENDER_PROCESS_H
#define RENDER_PROCESS_H

#include "content/content_export.h"
#include "content/process_impl.h"

namespace IPC{
class Channel;
}

namespace content {

class CONTENT_EXPORT RenderProcess : public ProcessImpl
{

public:
	RenderProcess(const QString& channel_name);
	virtual ~RenderProcess();

public:
	virtual bool Send(IPC::Message* msg);
	virtual bool OnMessageReceived(const IPC::Message& message);
	virtual void OnChannelConnected(int32_t peer_pid);

protected:
	IPC::Channel* channel_;
};


}

#endif // RENDER_PROCESS_H
