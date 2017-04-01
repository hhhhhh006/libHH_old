#ifndef ipc_channel_h__
#define ipc_channel_h__

#include <stdint.h>

#include "ipc_sender.h"

namespace IPC
{
	class Listener;

	class IPC_EXPORT Channel : public Sender
	{
	public:
		Channel() {}
		virtual ~Channel() {}

	public:
		enum Mode 
		{
			MODE_NONE   = 0x0,
			MODE_SERVER = 0x1,
			MODE_CLIENT = 0x2,
		};

		enum 
		{
			// The Hello message is sent by the peer when the channel is connected.
			// The message contains just the process id (pid).
			// The message has a special routing_id (MSG_ROUTING_NONE)
			// and type (HELLO_MESSAGE_TYPE).
			HELLO_MESSAGE_TYPE = UINT16_MAX,

			CLOSE_MESSAGE_TYPE = HELLO_MESSAGE_TYPE - 1
		};


		// The maximum message size in bytes. Attempting to receive a message of this
		// size or bigger results in a channel error.
		static const size_t kMaximumMessageSize = 128 * 1024 * 1024;

		// Amount of data to read at once from the pipe.
		static const size_t kReadBufferSize = 4 * 1024;

		// Maximum persistent read buffer size. Read buffer can grow larger to
		// accommodate large messages, but it's recommended to shrink back to this
		// value because it fits 99.9% of all messages (see issue 529940 for data).
		static const size_t kMaximumReadBufferSize = 64 * 1024;


		static Channel* Create(const QString& name, Mode mode, Listener* listener);

		static Channel* CreateClient(const QString& name, Listener* listener);

		static Channel* CreateServer(const QString& name, Listener* listener);


		virtual bool Connect() = 0;

		virtual void Close() = 0;

		// |message| must be allocated using operator new.  This object will be
		// deleted once the contents of the Message have been sent.
		virtual bool Send(Message* msg) = 0;

	};

}  // namespace IPC

#endif // ipc_channel_h__
