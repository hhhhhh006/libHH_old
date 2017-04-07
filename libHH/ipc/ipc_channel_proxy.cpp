// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ipc/ipc_channel_proxy.h"

#include <stddef.h>
#include <stdint.h>
#include <utility>

#include "base/compiler_specific.h"
#include "base/memory/ref_counted.h"
#include "base/single_thread_task_runner.h"
#include "base/threading/thread_task_runner_handle.h"
#include "build/build_config.h"
#include "ipc/ipc_listener.h"
#include "ipc/ipc_message_macros.h"
#include "ipc/message_filter.h"
#include "ipc/message_filter_router.h"

namespace IPC {

//------------------------------------------------------------------------------

ChannelProxy::Context::Context(
    Listener* listener,
    const scoped_refptr<base::SingleThreadTaskRunner>& ipc_task_runner)
    : listener_task_runner_(base::ThreadTaskRunnerHandle::Get()),
      listener_(listener),
      ipc_task_runner_(ipc_task_runner),
      channel_connected_called_(false),
      message_filter_router_(new MessageFilterRouter()),
      peer_pid_(base::kNullProcessId)
{

}

ChannelProxy::Context::~Context() {
}

void ChannelProxy::Context::ClearIPCTaskRunner()
{
    ipc_task_runner_ = NULL;
}

void ChannelProxy::Context::CreateChannel(const IPC::ChannelHandle& channel_handle, Channel::Mode mode)
{
    base::AutoLock lock(channel_lifetime_lock_);

    channel_ = Channel::Create(channel_handle, mode, this);
}

bool ChannelProxy::Context::TryFilters(const Message& message)
{
    if (message_filter_router_->TryFilters(message))
        return true;

    return false;
}

// Called on the IPC::Channel thread
bool ChannelProxy::Context::OnMessageReceived(const Message& message) 
{
    // First give a chance to the filters to process this message.
    if (!TryFilters(message))
        OnMessageReceivedNoFilter(message);
    return true;
}

// Called on the IPC::Channel thread
bool ChannelProxy::Context::OnMessageReceivedNoFilter(const Message& message) 
{
    base::Closure task = std::bind(&Context::OnDispatchMessage, this, message);
    listener_task_runner_->PostTask(task);
    return true;
}

// Called on the IPC::Channel thread
void ChannelProxy::Context::OnChannelConnected(int32_t peer_pid) 
{
    // We cache off the peer_pid so it can be safely accessed from both threads.
    peer_pid_ = channel_->GetPeerPID();

    OnAddFilter();

    // See above comment about using listener_task_runner_ here.
    base::Closure task = std::bind(&Context::OnDispatchConnected, this);
    listener_task_runner_->PostTask(task);
}

// Called on the IPC::Channel thread
void ChannelProxy::Context::OnChannelError() 
{
    base::Closure task = std::bind(&Context::OnDispatchError, this);
    listener_task_runner_->PostTask(task);
}

// Called on the IPC::Channel thread
void ChannelProxy::Context::OnChannelOpened() 
{
    // Assume a reference to ourselves on behalf of this thread.  This reference
    // will be released when we are closed.
    AddRef();

    if (!channel_->Connect())
    {
        OnChannelError();
        return;
    }

    for (size_t i = 0; i < filters_.size(); ++i)
        filters_[i]->OnFilterAdded(channel_.get());
}

// Called on the IPC::Channel thread
void ChannelProxy::Context::OnChannelClosed()
{
    if (!channel_)
        return;

    for (size_t i = 0; i < filters_.size(); ++i) 
    {
        filters_[i]->OnFilterRemoved();
    }

    // We don't need the filters anymore.
    message_filter_router_->Clear();
    filters_.clear();
    // We don't need the lock, because at this point, the listener thread can't
    // access it any more.
    pending_filters_.clear();

    ClearChannel();

    // Balance with the reference taken during startup.  This may result in
    // self-destruction.
    Release();
}

void ChannelProxy::Context::Clear()
{
    listener_ = NULL;
}

// Called on the IPC::Channel thread
void ChannelProxy::Context::OnSendMessage(Message* message)
{
    if (!channel_) 
    {
        OnChannelClosed();
        return;
    }

    if (!channel_->Send(message))
        OnChannelError();
}

// Called on the IPC::Channel thread
void ChannelProxy::Context::OnAddFilter()
{
    // Our OnChannelConnected method has not yet been called, so we can't be
    // sure that channel_ is valid yet. When OnChannelConnected *is* called,
    // it invokes OnAddFilter, so any pending filter(s) will be added at that
    // time.
    if (peer_pid_ == base::kNullProcessId)
        return;

    std::vector<scoped_refptr<MessageFilter> > new_filters;
    {
        base::AutoLock auto_lock(pending_filters_lock_);
        new_filters.swap(pending_filters_);
    }

    for (size_t i = 0; i < new_filters.size(); ++i) 
    {
        filters_.push_back(new_filters[i]);

        message_filter_router_->AddFilter(new_filters[i].get());

        // The channel has already been created and connected, so we need to
        // inform the filters right now.
        new_filters[i]->OnFilterAdded(channel_.get());
        new_filters[i]->OnChannelConnected(peer_pid_);
    }
}

// Called on the IPC::Channel thread
void ChannelProxy::Context::OnRemoveFilter(MessageFilter* filter)
{
    if (peer_pid_ == base::kNullProcessId) 
    {
        // The channel is not yet connected, so any filters are still pending.
        base::AutoLock auto_lock(pending_filters_lock_);
        for (size_t i = 0; i < pending_filters_.size(); ++i)
        {
            if (pending_filters_[i].get() == filter)
            {
                filter->OnFilterRemoved();
                pending_filters_.erase(pending_filters_.begin() + i);
                return;
            }
        }
        return;
    }

    if (!channel_)
        return;  // The filters have already been deleted.

    message_filter_router_->RemoveFilter(filter);

    for (size_t i = 0; i < filters_.size(); ++i)
    {
        if (filters_[i].get() == filter)
        {
            filter->OnFilterRemoved();
            filters_.erase(filters_.begin() + i);
            return;
        }
    }

}

// Called on the listener's thread
void ChannelProxy::Context::AddFilter(MessageFilter* filter)
{
    base::AutoLock auto_lock(pending_filters_lock_);
    pending_filters_.push_back(make_scoped_refptr(filter));
    
    base::Closure task = std::bind(&Context::OnAddFilter, this);
    ipc_task_runner_->PostTask(task);
}

// Called on the listener's thread
void ChannelProxy::Context::OnDispatchMessage(const Message& message)
{
    if (!listener_)
        return;

    OnDispatchConnected();

    listener_->OnMessageReceived(message);
}

// Called on the listener's thread
void ChannelProxy::Context::OnDispatchConnected()
{
    if (channel_connected_called_)
        return;

    channel_connected_called_ = true;
    if (listener_)
        listener_->OnChannelConnected(peer_pid_);
}

// Called on the listener's thread
void ChannelProxy::Context::OnDispatchError() 
{
    if (listener_)
        listener_->OnChannelError();
}

// Called on the listener's thread
void ChannelProxy::Context::OnDispatchBadMessage(const Message& message)
{
    if (listener_)
        listener_->OnBadMessageReceived(message);
}

void ChannelProxy::Context::ClearChannel()
{
    base::AutoLock lock(channel_lifetime_lock_);
    channel_.reset();
}


void ChannelProxy::Context::Send(Message* message)
{
  //  std::unique_ptr<Message> msg(message);
    base::Closure task = std::bind(&ChannelProxy::Context::OnSendMessage, this, message);
    ipc_task_runner()->PostTask(task);
}


//-----------------------------------------------------------------------------

// static
std::unique_ptr<ChannelProxy> ChannelProxy::Create(
    const IPC::ChannelHandle& channel_handle,
    Channel::Mode mode,
    Listener* listener,
    const scoped_refptr<base::SingleThreadTaskRunner>& ipc_task_runner) 
{
    std::unique_ptr<ChannelProxy> channel(new ChannelProxy(listener, ipc_task_runner));
    channel->Init(channel_handle, mode);
    return channel;
}


ChannelProxy::ChannelProxy(Listener* listener,
    const scoped_refptr<base::SingleThreadTaskRunner>& ipc_task_runner)
    : context_(new Context(listener, ipc_task_runner))
{

}

ChannelProxy::~ChannelProxy()
{
    Close();
}

void ChannelProxy::Init(const IPC::ChannelHandle& channel_handle,
                        Channel::Mode mode)
{
    context_->CreateChannel(channel_handle, mode);

    // complete initialization on the background thread
    base::Closure task = std::bind(&Context::OnChannelOpened, context_.get());
    context_->ipc_task_runner()->PostTask(task);
}


void ChannelProxy::Close() 
{
    context_->Clear();

    if (context_->ipc_task_runner()) 
    {
        base::Closure task = std::bind(&Context::OnChannelClosed, context_.get());
        context_->ipc_task_runner()->PostTask(task);
    }
}

bool ChannelProxy::Send(Message* message) 
{
    context_->Send(message);
    return true;
}

void ChannelProxy::AddFilter(MessageFilter* filter) 
{
    context_->AddFilter(filter);
}

void ChannelProxy::RemoveFilter(MessageFilter* filter)
{
    base::Closure task = std::bind(&Context::OnRemoveFilter, context_.get(), filter);
    context_->ipc_task_runner()->PostTask(task);
}

void ChannelProxy::ClearIPCTaskRunner()
{
    context_->ClearIPCTaskRunner();
}

base::ProcessId ChannelProxy::GetPeerPID() const 
{
    return context_->peer_pid_;
}


//-----------------------------------------------------------------------------

}  // namespace IPC
