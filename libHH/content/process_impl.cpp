#include "process_impl.h"
#include "IPC/message_filter_router.h"
#include "IPC/message_filter.h"

namespace content {

ProcessImpl::ProcessImpl()
{
	message_filter_router_ = new IPC::MessageFilterRouter;
}

ProcessImpl::~ProcessImpl()
{
	delete message_filter_router_;
}


void ProcessImpl::AddFilter(IPC::MessageFilter* filter)
{
	message_filter_router_->AddFilter(filter);
	filter->OnFilterAdded(this);
}

void ProcessImpl::RemoveFilter(IPC::MessageFilter* filter)
{
	message_filter_router_->RemoveFilter(filter);
	filter->OnFilterRemoved();
}


bool ProcessImpl::TryFilters(const IPC::Message& message)
{
	return message_filter_router_->TryFilters(message);
}

}