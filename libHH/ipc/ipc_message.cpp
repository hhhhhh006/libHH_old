#include "ipc_message.h"


namespace IPC {

Message::Message()
	: base::Pickle(sizeof(Header))
{
	header()->routing = 0;
	header()->type = 0;
	header()->flags = 0;

	sender_pid_ = 0;
}

Message::Message(int32_t routing_id, uint32_t type)
	: base::Pickle(sizeof(Header))
{
	header()->routing = routing_id;
	header()->type = type;
	header()->flags = 0;

	sender_pid_ = 0;
}

Message::Message(const char* data, int data_len)
	: base::Pickle(data, data_len)
{
	sender_pid_ = 0;
}

Message::Message(const Message& other)
	: base::Pickle(other)
{
	sender_pid_ = other.sender_pid_;
}

Message& Message::operator=(const Message& other)
{
	*static_cast<base::Pickle*>(this) = other;
	return *this;
}

Message::~Message()
{
}

void Message::SetHeaderValues(int32_t routing, uint32_t type, uint32_t flags)
{
	header()->routing = routing;
	header()->type = type;
	header()->flags = flags;
}

void Message::FindNext(const char* range_start, const char* range_end, NextMessageInfo* info)
{
	info->message_found = false;
	info->message_size = 0;

	size_t pickle_size = 0;
	if (!base::Pickle::PeekNext(sizeof(Header), range_start, range_end, &pickle_size))
		return;

	info->message_size = pickle_size;

	bool have_entire_pickle = static_cast<size_t>(range_end - range_start) >= pickle_size;
	if (!have_entire_pickle)
		return;

	const char* pickle_end = range_start + pickle_size;
	info->message_end = pickle_end;
	info->pickle_end = pickle_end;
	info->message_found = true;
}

Message::NextMessageInfo::NextMessageInfo()
	: message_size(0),
	message_found(false),
	pickle_end(nullptr),
	message_end(nullptr)
{

}

Message::NextMessageInfo::~NextMessageInfo()
{

}

}


