#ifndef ipc_channel_reader_h__
#define ipc_channel_reader_h__

#include <string>
#include <vector>

#include "base/memory/scoped_vector.h"
#include "ipc/ipc_channel.h"
#include "ipc/ipc_message.h"


namespace IPC {
class Listener;

namespace internal {

class IPC_EXPORT ChannelReader	
{
		
public:
	explicit ChannelReader(Listener* listener);
	virtual ~ChannelReader();

public:
    void set_listener(Listener* listener) { listener_ = listener; }

    // This type is returned by ProcessIncomingMessages to indicate the effect of
    // the method.
    enum DispatchState {
        // All messages were successfully dispatched, or there were no messages to
        // dispatch.
        DISPATCH_FINISHED,
        // There was a channel error.
        DISPATCH_ERROR,
        // Dispatching messages is blocked on receiving more information from the
        // broker.
        DISPATCH_WAITING_ON_BROKER,
    };

    // Call to process messages received from the IPC connection and dispatch
    // them.
    DispatchState ProcessIncomingMessages();

    // Handles asynchronously read data.
    //
    // Optionally call this after returning READ_PENDING from ReadData to
    // indicate that buffer was filled with the given number of bytes of
    // data. See ReadData for more.
    DispatchState AsyncReadComplete(int bytes_read);

    bool IsInternalMessage(const Message& msg);

    bool IsHelloMessage(const Message& msg);

protected:
    enum ReadState { READ_SUCCEEDED, READ_FAILED, READ_PENDING };

    Listener* listener() const { return listener_; }

    // Populates the given buffer with data from the pipe.
    //
    // Returns the state of the read. On READ_SUCCESS, the number of bytes
    // read will be placed into |*bytes_read| (which can be less than the
    // buffer size). On READ_FAILED, the channel will be closed.
    //
    // If the return value is READ_PENDING, it means that there was no data
    // ready for reading. The implementation is then responsible for either
    // calling AsyncReadComplete with the number of bytes read into the
    // buffer, or ProcessIncomingMessages to try the read again (depending
    // on whether the platform's async I/O is "try again" or "write
    // asynchronously into your buffer").
    virtual ReadState ReadData(char* buffer, int buffer_len, int* bytes_read) = 0;

	virtual void HandleInternalMessage(const Message& msg) = 0;

    virtual void DispatchMessage(Message* msg);

    virtual base::ProcessId GetSenderPID() = 0;

private:
	bool TranslateInputData(const char* input_data, int input_data_len);
	
    bool HandleTranslatedMessage(Message* translated_message);
	
    DispatchState DispatchMessages();
	
	bool CheckMessageSize(size_t size);

private:
	Listener* listener_;

	char input_buf_[Channel::kReadBufferSize];

	std::string input_overflow_buf_;

	size_t max_input_buffer_size_;

	ScopedVector<Message> queued_messages_;

};

} // namespace internal

} // namespace IPC

#endif // ipc_channel_reader_h__
