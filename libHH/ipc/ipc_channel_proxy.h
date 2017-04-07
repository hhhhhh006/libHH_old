// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IPC_IPC_CHANNEL_PROXY_H_
#define IPC_IPC_CHANNEL_PROXY_H_

#include <stdint.h>

#include <vector>

#include "base/memory/ref_counted.h"
#include "base/synchronization/lock.h"
#include "ipc/ipc_channel.h"
#include "ipc/ipc_listener.h"
#include "ipc/ipc_sender.h"

namespace base {
class SingleThreadTaskRunner;
}

namespace IPC {
class MessageFilter;
class MessageFilterRouter;


// ChannelProxy 负责把IPC消息运行在IO后台线程执行,并把Listener接口代理回主线程

// MessageFilter 的回调接口生存在IO线程
// MessageFilter 如果处理了IPC消息，不再发送消息到Listener接口


class IPC_EXPORT ChannelProxy : public Sender
{
public:
    static std::unique_ptr<ChannelProxy> Create(
        const IPC::ChannelHandle& channel_handle,
        Channel::Mode mode,
        Listener* listener,
        const scoped_refptr<base::SingleThreadTaskRunner>& ipc_task_runner);

    ~ChannelProxy();

    // Initializes the channel proxy. Only call this once to initialize a channel
    // proxy that was not initialized in its constructor. If create_pipe_now is
    // true, the pipe is created synchronously. Otherwise it's created on the IO
    // thread.
    void Init(const IPC::ChannelHandle& channel_handle, Channel::Mode mode);

    // Close the IPC::Channel.  This operation completes asynchronously, once the
    // background thread processes the command to close the channel.  It is ok to
    // call this method multiple times.  Redundant calls are ignored.
    //
    // WARNING: MessageFilter objects held by the ChannelProxy is also
    // released asynchronously, and it may in fact have its final reference
    // released on the background thread.  The caller should be careful to deal
    // with / allow for this possibility.
    void Close();

    // Send a message asynchronously.  The message is routed to the background
    // thread where it is passed to the IPC::Channel's Send method.
    bool Send(Message* message) override;

    // Used to intercept messages as they are received on the background thread.
    //
    // Ordinarily, messages sent to the ChannelProxy are routed to the matching
    // listener on the worker thread.  This API allows code to intercept messages
    // before they are sent to the worker thread.
    // If you call this before the target process is launched, then you're
    // guaranteed to not miss any messages.  But if you call this anytime after,
    // then some messages might be missed since the filter is added internally on
    // the IO thread.
    void AddFilter(MessageFilter* filter);
    void RemoveFilter(MessageFilter* filter);


    // Called to clear the pointer to the IPC task runner when it's going away.
    void ClearIPCTaskRunner();

    base::ProcessId GetPeerPID() const;


protected:
    class Context;
    // A subclass uses this constructor if it needs to add more information
    // to the internal state.

    ChannelProxy(Listener* listener,
        const scoped_refptr<base::SingleThreadTaskRunner>& ipc_task_runner);

    // Used internally to hold state that is referenced on the IPC thread.
    class Context : public base::RefCountedThreadSafe<Context>, public Listener 
    {
    public:
        Context(Listener* listener,
            const scoped_refptr<base::SingleThreadTaskRunner>& ipc_thread);

        base::SingleThreadTaskRunner* ipc_task_runner() const {
            return ipc_task_runner_.get();
        }

        void ClearIPCTaskRunner();

        const std::string& channel_id() const { return channel_id_; }

        // Dispatches a message on the listener thread.
        void OnDispatchMessage(const Message& message);

        // Sends |message| from appropriate thread.
        void Send(Message* message);


    protected:
        friend class base::RefCountedThreadSafe<Context>;
        ~Context();

        // IPC::Listener methods:
        bool OnMessageReceived(const Message& message) override;
        void OnChannelConnected(int32_t peer_pid) override;
        void OnChannelError() override;

        // Like OnMessageReceived but doesn't try the filters.
        bool OnMessageReceivedNoFilter(const Message& message);

        // Gives the filters a chance at processing |message|.
        // Returns true if the message was processed, false otherwise.
        bool TryFilters(const Message& message);

        // Like Open and Close, but called on the IPC thread.
        virtual void OnChannelOpened();
        virtual void OnChannelClosed();

        // Called on the consumers thread when the ChannelProxy is closed.  At that
        // point the consumer is telling us that they don't want to receive any
        // more messages, so we honor that wish by forgetting them!
        virtual void Clear();

    private:
        friend class ChannelProxy;
        friend class IpcSecurityTestUtil;

        // Create the Channel
        void CreateChannel(const IPC::ChannelHandle& channel_handle, Channel::Mode mode);

        // Methods called on the IO thread.
        void OnSendMessage(Message* message);
        void OnAddFilter();
        void OnRemoveFilter(MessageFilter* filter);

        // Methods called on the listener thread.
        void AddFilter(MessageFilter* filter);
        void OnDispatchConnected();
        void OnDispatchError();
        void OnDispatchBadMessage(const Message& message);

        void ClearChannel();

        scoped_refptr<base::SingleThreadTaskRunner> listener_task_runner_;
        Listener* listener_;

        // List of filters.  This is only accessed on the IPC thread.
        std::vector<scoped_refptr<MessageFilter> > filters_;
        scoped_refptr<base::SingleThreadTaskRunner> ipc_task_runner_;

        // Note, channel_ may be set on the Listener thread or the IPC thread.
        // But once it has been set, it must only be read or cleared on the IPC
        // thread.
        // One exception is the thread-safe send. See the class comment.
        std::unique_ptr<Channel> channel_;
        std::string channel_id_;
        bool channel_connected_called_;

        // Lock for |channel_| value. This is only relevant in the context of
        // thread-safe send.
        base::Lock channel_lifetime_lock_;

        // Routes a given message to a proper subset of |filters_|, depending
        // on which message classes a filter might support.
        std::unique_ptr<MessageFilterRouter> message_filter_router_;

        // Holds filters between the AddFilter call on the listerner thread and the
        // IPC thread when they're added to filters_.
        std::vector<scoped_refptr<MessageFilter> > pending_filters_;
        // Lock for pending_filters_.
        base::Lock pending_filters_lock_;

        // Cached copy of the peer process ID. Set on IPC but read on both IPC and
        // listener threads.
        base::ProcessId peer_pid_;
    };

private:
    scoped_refptr<Context> context_;

};

}  // namespace IPC

#endif  // IPC_IPC_CHANNEL_PROXY_H_
