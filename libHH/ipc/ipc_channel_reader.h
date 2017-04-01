#ifndef ipc_channel_reader_h__
#define ipc_channel_reader_h__

#include <string>
#include <vector>

#include "ipc/ipc_channel.h"


namespace IPC {
class Listener;

class IPC_EXPORT ChannelReader	
{
		
public:
	explicit ChannelReader(Listener* listener);
	virtual ~ChannelReader();

public:
	Listener* listener() const { return listener_; }

	void InputData(const char* data, int data_len);

	bool GetHelloMessage(Message* message);

	bool IsInternalMessage(const Message& m);

protected:
	virtual void HandleInternalMessage(const Message& msg) {}

private:
	bool TranslateInputData();
	bool HandleTranslatedMessage(Message* translated_message);
	void DispatchMessage(Message* msg);
	void DispatchMessages();
	bool CheckMessageSize(size_t size);

private:
	Listener* listener_;

	char input_buf_[Channel::kReadBufferSize];

	std::string input_overflow_buf_;

	size_t max_input_buffer_size_;

	std::vector<Message*> queued_messages_;

	Message* hello_message_;

};

}

#endif // ipc_channel_reader_h__
