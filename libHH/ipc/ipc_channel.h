#ifndef ipc_channel_h__
#define ipc_channel_h__

#include <stdint.h>

#include <memory>

#include "ipc/ipc_sender.h"
#include "ipc/ipc_message.h"


namespace base {
typedef int ProcessId;
const ProcessId kNullProcessId = 0;
}

namespace IPC {

class Listener;

typedef std::string ChannelHandle;


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


    static std::unique_ptr<Channel> Create(const ChannelHandle& channel_handle, Mode mode, Listener* listener);

    static std::unique_ptr<Channel> CreateServer(const ChannelHandle& channel_handle, Listener* listener);

    static std::unique_ptr<Channel> CreateClient(const ChannelHandle& channel_handle, Listener* listener);


    virtual bool Connect() = 0;

    virtual void Close() = 0;

    // |message| must be allocated using operator new.  This object will be
    // deleted once the contents of the Message have been sent.
    virtual bool Send(Message* msg) = 0;

    virtual base::ProcessId GetPeerPID() const = 0;
    virtual base::ProcessId GetSelfPID() const = 0;

    // Returns true if a named server channel is initialized on the given channel
    // ID. Even if true, the server may have already accepted a connection.
    static bool IsNamedServerInitialized(const std::string& channel_id);

protected:
    // An OutputElement is a wrapper around a Message or raw buffer while it is
    // waiting to be passed to the system's underlying IPC mechanism.
    class OutputElement
    {
    public:
        // Takes ownership of message.
        OutputElement(Message* message);
        // Takes ownership of the buffer. |buffer| is freed via free(), so it
        // must be malloced.
        OutputElement(void* buffer, size_t length);
        ~OutputElement();
        size_t size() const { return message_ ? message_->size() : length_; }
        const void* data() const { return message_ ? message_->data() : buffer_; }
        Message* get_message() const { return message_.get(); }

    private:
        std::unique_ptr<Message> message_;
        void* buffer_;
        size_t length_;
    };

};


}  // namespace IPC

#endif // ipc_channel_h__
