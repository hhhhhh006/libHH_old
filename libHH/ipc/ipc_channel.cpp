#include "ipc_channel.h"

namespace IPC {

	Channel* Channel::CreateServer(const QString& name, Listener* listener)
	{
		return Create(name, Channel::MODE_SERVER, listener);
	}

	Channel* Channel::CreateClient(const QString& name, Listener* listener)
	{
		return Create(name, Channel::MODE_CLIENT, listener);
	}
}


