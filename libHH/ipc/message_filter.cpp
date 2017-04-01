#include "message_filter.h"

#include <stdint.h>

#include "ipc_channel.h"

namespace IPC {

MessageFilter::MessageFilter() {}

void MessageFilter::OnFilterAdded(Sender* sender) {}

void MessageFilter::OnFilterRemoved() {}


bool MessageFilter::OnMessageReceived(const Message& message) 
{
	return false;
}

bool MessageFilter::GetSupportedMessageClasses(std::vector<uint32_t>* /*supported_message_classes*/) const 
{
	return false;
}

MessageFilter::~MessageFilter() {}

}  // namespace IPC
