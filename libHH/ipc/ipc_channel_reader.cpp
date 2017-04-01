#include "ipc_channel_reader.h"
#include "ipc_listener.h"
#include "ipc_message.h"

namespace IPC {
		
ChannelReader::ChannelReader(Listener* listener)
	: listener_(listener),
	  max_input_buffer_size_(Channel::kMaximumReadBufferSize),
	  hello_message_(nullptr)
{
	memset(input_buf_, 0, sizeof(input_buf_));
}


ChannelReader::~ChannelReader()
{
}


void ChannelReader::InputData(const char* data, int data_len)
{
	input_overflow_buf_.append(data, data_len);
	TranslateInputData();
	DispatchMessages();
}

bool ChannelReader::GetHelloMessage(Message* message)
{
	if (hello_message_)
	{
		*message = *hello_message_;
		delete hello_message_;
		hello_message_ = nullptr;
		return true;
	}
	return false;
}

bool ChannelReader::IsInternalMessage(const Message& msg)
{
	return msg.routing_id() == MSG_ROUTING_NONE && 
		   msg.type() == Channel::HELLO_MESSAGE_TYPE;
}

bool ChannelReader::TranslateInputData()
{
	if (input_overflow_buf_.empty())
		return false;

	if (!CheckMessageSize(input_overflow_buf_.size()))
		return false;
	
	const char* p   = input_overflow_buf_.data();
	const char* end = p + input_overflow_buf_.size();

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
		hello_message_ = new Message(*translated_message);
		HandleInternalMessage(*translated_message);
		return true;
	}

	Message *m = new Message(*translated_message);
	queued_messages_.push_back(m);
	return true;
}

void ChannelReader::DispatchMessage(Message* msg)
{
	listener_->OnMessageReceived(*msg);
	delete msg;
}

void ChannelReader::DispatchMessages()
{
	while (!queued_messages_.empty())
	{
		Message *m = queued_messages_.front();
		queued_messages_.erase(queued_messages_.begin());
		DispatchMessage(m);
	}
}

bool ChannelReader::CheckMessageSize(size_t size)
{
	if (size <= Channel::kMaximumMessageSize)
		return true;

	input_overflow_buf_.clear();
	return false;
}


}


