#ifndef IPC_LISTENER_H__
#define IPC_LISTENER_H__

#include <stdint.h>

#include "ipc/ipc_export.h"

namespace IPC 
{
	
class Message;

class IPC_EXPORT Listener
{
public:
	Listener() {}
	virtual ~Listener() {}

public:
	virtual bool OnMessageReceived(const Message& message) = 0;
	
	virtual void OnChannelConnected(int32_t peer_pid) {}

	virtual void OnChannelError() {}
	
	virtual void OnBadMessageReceived(const Message& message) {}
	  
};
	
}

#endif
