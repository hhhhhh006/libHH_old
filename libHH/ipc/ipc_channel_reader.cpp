#include "ipc/ipc_channel_reader.h"
#include "ipc/ipc_listener.h"
#include "ipc/ipc_message.h"

namespace IPC {
	
namespace internal {

ChannelReader::ChannelReader(Listener* listener)
	: listener_(listener),
	  max_input_buffer_size_(Channel::kMaximumReadBufferSize)
{
	memset(input_buf_, 0, sizeof(input_buf_));
}


ChannelReader::~ChannelReader()
{
}

ChannelReader::DispatchState ChannelReader::ProcessIncomingMessages()
{
    while (true) 
    {
        int bytes_read = 0;
        ReadState read_state = ReadData(input_buf_, Channel::kReadBufferSize, &bytes_read);
        if (read_state == READ_FAILED)
            return DISPATCH_ERROR;
        if (read_state == READ_PENDING)
            return DISPATCH_FINISHED;

        if (!TranslateInputData(input_buf_, bytes_read))
            return DISPATCH_ERROR;

        DispatchState state = DispatchMessages();
        if (state != DISPATCH_FINISHED)
            return state;
    }
}

ChannelReader::DispatchState ChannelReader::AsyncReadComplete(int bytes_read)
{
    if (!TranslateInputData(input_buf_, bytes_read))
        return DISPATCH_ERROR;

    return DispatchMessages();
}


bool ChannelReader::IsInternalMessage(const Message& msg)
{
    return msg.routing_id() == MSG_ROUTING_NONE && 
        msg.type() == Channel::HELLO_MESSAGE_TYPE;
}

bool ChannelReader::IsHelloMessage(const Message& msg)
{
	return msg.routing_id() == MSG_ROUTING_NONE && 
		   msg.type() == Channel::HELLO_MESSAGE_TYPE;
}

bool ChannelReader::TranslateInputData(const char* input_data, int input_data_len)
{
    const char* p;
    const char* end;

    if (input_overflow_buf_.empty()) 
    {
        p = input_data;
        end = input_data + input_data_len;
    } 
    else 
    {
        if (!CheckMessageSize(input_overflow_buf_.size() + input_data_len))
            return false;

        input_overflow_buf_.append(input_data, input_data_len);
        p = input_overflow_buf_.data();
        end = p + input_overflow_buf_.size();
    }

	size_t next_message_size = 0;

	while (p < end)
	{
		Message::NextMessageInfo info;
		Message::FindNext(p, end, &info);
		if (info.message_found)
		{
			int pickle_len = static_cast<int>(info.pickle_end - p);
			Message translated_message(p, pickle_len);

			if (!HandleTranslatedMessage(&translated_message))
				return false;

			p = info.message_end;
		}
		else
		{
			next_message_size = info.message_size;
			if (!CheckMessageSize(next_message_size))
				return false;
			break;
		}
	}

	size_t next_message_buffer_size = next_message_size ? 
		next_message_size + Channel::kReadBufferSize - 1 : 0;

	// Save any partial data in the overflow buffer.
	if (p != input_overflow_buf_.data())
	{
		input_overflow_buf_.assign(p, end - p);
	}

	if (!input_overflow_buf_.empty())
	{
		if (next_message_buffer_size > input_overflow_buf_.capacity())
			input_overflow_buf_.reserve(next_message_buffer_size);
	}

	if (next_message_buffer_size < max_input_buffer_size_ &&
		input_overflow_buf_.size() < max_input_buffer_size_ &&
		input_overflow_buf_.capacity() > max_input_buffer_size_)
	{
		std::string trimmed_buf;
		trimmed_buf.reserve(max_input_buffer_size_);
		if (trimmed_buf.capacity() > max_input_buffer_size_)
		{
			max_input_buffer_size_ = trimmed_buf.capacity();
		}
		trimmed_buf.assign(input_overflow_buf_.data(), input_overflow_buf_.size());
		input_overflow_buf_.swap(trimmed_buf);
	}
	
	return true;
}

bool ChannelReader::HandleTranslatedMessage(Message* translated_message)
{
	if (IsInternalMessage(*translated_message))
	{
		HandleInternalMessage(*translated_message);
		return true;
	}

    translated_message->set_sender_pid(GetSenderPID());

	std::unique_ptr<Message> m(new Message(*translated_message));
	queued_messages_.push_back(m.release());
	return true;
}

void ChannelReader::DispatchMessage(Message* msg)
{
	listener_->OnMessageReceived(*msg);
}

ChannelReader::DispatchState ChannelReader::DispatchMessages()
{
	while (!queued_messages_.empty())
	{
		Message *m = queued_messages_.front();
		DispatchMessage(m);
        queued_messages_.erase(queued_messages_.begin());
	}
    return DISPATCH_FINISHED;
}

bool ChannelReader::CheckMessageSize(size_t size)
{
	if (size <= Channel::kMaximumMessageSize)
		return true;

	input_overflow_buf_.clear();
	return false;
}


}  // namespace internal

} // namespace IPC


