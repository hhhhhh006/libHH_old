#ifndef PROCESS_IMPL_H
#define PROCESS_IMPL_H

#include "ipc/ipc_sender.h"
#include "ipc/ipc_listener.h"
#include "content/content_export.h"

namespace IPC {
class MessageFilter;
class MessageFilterRouter;
}

namespace content {

class CONTENT_EXPORT ProcessImpl : public IPC::Sender,
								   public IPC::Listener
{

public:
	ProcessImpl();
	virtual ~ProcessImpl();

	void AddFilter(IPC::MessageFilter* filter);
	void RemoveFilter(IPC::MessageFilter* filter);
	bool TryFilters(const IPC::Message& message);

protected:
	IPC::MessageFilterRouter* message_filter_router_;

};

}

#endif // PROCESS_IMPL_H
