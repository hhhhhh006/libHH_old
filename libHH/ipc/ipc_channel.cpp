#include "ipc/ipc_channel.h"


namespace IPC {

std::unique_ptr<Channel> Channel::CreateServer(const ChannelHandle& channel_handle, Listener* listener)
{
	return Channel::Create(channel_handle, Channel::MODE_SERVER, listener);
}

std::unique_ptr<Channel> Channel::CreateClient(const ChannelHandle& channel_handle, Listener* listener)
{
	return Channel::Create(channel_handle, Channel::MODE_CLIENT, listener);
}


Channel::OutputElement::OutputElement(Message* message)
    : message_(message), buffer_(nullptr), length_(0) {}

Channel::OutputElement::OutputElement(void* buffer, size_t length)
    : message_(nullptr), buffer_(buffer), length_(length) {}

Channel::OutputElement::~OutputElement() {
    free(buffer_);
}


}


