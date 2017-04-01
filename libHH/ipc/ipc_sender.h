#ifndef IPC_SENDER_H_
#define IPC_SENDER_H_

#include "ipc/ipc_export.h"

namespace IPC 
{
class Message;

class IPC_EXPORT Sender 
{
public:
	Sender() {}
	virtual ~Sender() {}

public:
	virtual bool Send(Message* msg) = 0;
		
};

}

#endif // IPC_SENDER_H_
