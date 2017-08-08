// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ipc/ipc_channel_win.h"

#include <windows.h>
#include <stddef.h>
#include <stdint.h>

#include "base/callback.h"
#include "base/compiler_specific.h"
#include "base/pickle.h"
#include "base/win/scoped_handle.h"
#include "ipc/ipc_listener.h"
#include "ipc/ipc_message_utils.h"


namespace IPC {

ChannelWin::State::State(ChannelWin* channel) : is_pending(false) 
{
    memset(&context.overlapped, 0, sizeof(context.overlapped));
}

ChannelWin::State::~State() 
{

}

ChannelWin::ChannelWin(const IPC::ChannelHandle& channel_handle, Mode mode,
                       Listener* listener)
    : ChannelReader(listener),
      input_state_(this),
      output_state_(this),
      peer_pid_(base::kNullProcessId),
      waiting_connect_(mode & MODE_SERVER),
      weak_factory_(this) 
{
    CreatePipe(channel_handle, mode);
}

ChannelWin::~ChannelWin() 
{
    Close();
}

void ChannelWin::Close() 
{
    if (input_state_.is_pending || output_state_.is_pending)
        CancelIo(pipe_.Get());

    // Closing the handle at this point prevents us from issuing more requests
    // form OnIOCompleted().
    if (pipe_.IsValid())
        pipe_.Close();

    // Make sure all IO has completed.
    while (input_state_.is_pending || output_state_.is_pending) 
    {
        base::MessageLoopForIO::current()->WaitForIOCompletion(INFINITE, this);
    }

    while (!output_queue_.empty()) 
    {
        OutputElement* element = output_queue_.front();
        output_queue_.pop();
        delete element;
    }
}

bool ChannelWin::Send(Message* message) 
{
    if (!prelim_queue_.empty()) 
    {
        prelim_queue_.push(message);
        return true;
    }

    if (peer_pid_ == base::kNullProcessId) 
    {
        prelim_queue_.push(message);
        return true;
    }

    return ProcessMessageForDelivery(message);
}

bool ChannelWin::ProcessMessageForDelivery(Message* message)
{
    // OutputElement 管理 Message 的生存期
    OutputElement* element = new OutputElement(message);
    output_queue_.push(element);

    // ensure waiting to write
    if (!waiting_connect_) 
    {
        if (!output_state_.is_pending) 
        {
            if (!ProcessOutgoingMessages(NULL, 0))
                return false;
        }
    }

    return true;
}

void ChannelWin::FlushPrelimQueue() 
{
    // 清空管道没连接之前，缓存发送的Message
    std::queue<Message*> prelim_queue;
    prelim_queue_.swap(prelim_queue);

    while (!prelim_queue.empty()) 
    {
        Message* m = prelim_queue.front();
        bool success = ProcessMessageForDelivery(m);
        prelim_queue.pop();

        if (!success)
            break;
    }

    // Delete any unprocessed messages.
    while (!prelim_queue.empty()) 
    {
        Message* m = prelim_queue.front();
        delete m;
        prelim_queue.pop();
    }
}


base::ProcessId ChannelWin::GetPeerPID() const 
{
    return peer_pid_;
}

base::ProcessId ChannelWin::GetSelfPID() const 
{
    return ::GetCurrentProcessId();
}

// static
bool ChannelWin::IsNamedServerInitialized(const std::string& channel_id) 
{
    if (WaitNamedPipeA(PipeName(channel_id).c_str(), 1))
        return true;
    // If ERROR_SEM_TIMEOUT occurred, the pipe exists but is handling another
    // connection.
    return GetLastError() == ERROR_SEM_TIMEOUT;
}

ChannelWin::ReadState ChannelWin::ReadData(char* buffer,
                                           int buffer_len,
                                           int* /* bytes_read */) 
{
    if (!pipe_.IsValid())
        return READ_FAILED;

    DWORD bytes_read = 0;
    BOOL ok = ReadFile(pipe_.Get(), buffer, buffer_len,
                       &bytes_read, &input_state_.context.overlapped);
    if (!ok) 
    {
        DWORD err = GetLastError();
        if (err == ERROR_IO_PENDING) 
        {
            input_state_.is_pending = true;
            return READ_PENDING;
        }
        return READ_FAILED;
    }

    input_state_.is_pending = true;
    return READ_PENDING;
}


void ChannelWin::HandleInternalMessage(const Message& msg) 
{
    // The hello message contains one parameter containing the PID.
    base::PickleIterator it(msg);
    int32_t claimed_pid;
    bool failed = !it.ReadInt(&claimed_pid);
    if (failed) 
    {
        Close();
        listener()->OnChannelError();
        return;
    }

    peer_pid_ = claimed_pid;

    listener()->OnChannelConnected(claimed_pid);

    FlushPrelimQueue();
}

base::ProcessId ChannelWin::GetSenderPID() 
{
    return GetPeerPID();
}

// static
const std::string ChannelWin::PipeName(const std::string& channel_id) 
{
    std::string name("\\\\.\\pipe\\libHH.");
    name.append(channel_id);
    return name;
}

bool ChannelWin::CreatePipe(const IPC::ChannelHandle &channel_handle, Mode mode) 
{
    std::string pipe_name;

    if (mode & MODE_SERVER) 
    {
        const DWORD open_mode = PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED |
                                FILE_FLAG_FIRST_PIPE_INSTANCE;
        pipe_name = PipeName(channel_handle);
        pipe_.Set(CreateNamedPipeA(pipe_name.c_str(),
                                   open_mode,
                                   PIPE_TYPE_BYTE | PIPE_READMODE_BYTE,
                                   1,
                                   Channel::kReadBufferSize,
                                   Channel::kReadBufferSize,
                                   5000,
                                   NULL));
    } 
    else if (mode & MODE_CLIENT) 
    {
        pipe_name = PipeName(channel_handle);
        pipe_.Set(CreateFileA(pipe_name.c_str(),
                              GENERIC_READ | GENERIC_WRITE,
                              0,
                              NULL,
                              OPEN_EXISTING,
                              SECURITY_SQOS_PRESENT | SECURITY_ANONYMOUS |
                              FILE_FLAG_OVERLAPPED,
                              NULL));
    } 
    int err = ::GetLastError();
    if (!pipe_.IsValid())
    {
        return false;
    }

    // 发送 Hello Message 类型，里面携带了进程PID
    // 管道双方确认接收到 Hello Message 消息，才认为连接成功
    std::unique_ptr<Message> m(new Message(MSG_ROUTING_NONE, HELLO_MESSAGE_TYPE));
    if (!m->WriteInt(::GetCurrentProcessId())) 
    {
        pipe_.Close();
        return false;
    }

    OutputElement* element = new OutputElement(m.release());
    output_queue_.push(element);
    return true;
}

bool ChannelWin::Connect() 
{
    if (!pipe_.IsValid())
        return false;

    base::MessageLoopForIO::current()->RegisterIOHandler(pipe_.Get(), this);

    // Check to see if there is a client connected to our pipe...
    if (waiting_connect_)
        ProcessConnection();

    if (!input_state_.is_pending) 
    {
        // Complete setup asynchronously. By not setting input_state_.is_pending
        // to true, we indicate to OnIOCompleted that this is the special
        // initialization signal.
        base::Closure task = std::bind(&ChannelWin::OnIOCompleted, weak_factory_.GetWeakPtr(),
                                       &input_state_.context, 0, 0);
        base::MessageLoopForIO::current()->PostTask(task);
    }

    if (!waiting_connect_)
        ProcessOutgoingMessages(NULL, 0);
    return true;
}

bool ChannelWin::ProcessConnection() 
{
    if (input_state_.is_pending)
        input_state_.is_pending = false;

    // Do we have a client connected to our pipe?
    if (!pipe_.IsValid())
        return false;

    BOOL ok = ConnectNamedPipe(pipe_.Get(), &input_state_.context.overlapped);
    if (ok) 
    {
        // Uhm, the API documentation says that this function should never
        // return success when used in overlapped mode.
        return false;
    }

    DWORD err = GetLastError();
    switch (err) 
    {
    case ERROR_IO_PENDING:
        input_state_.is_pending = true;
        break;
    case ERROR_PIPE_CONNECTED:
        waiting_connect_ = false;
        break;
    case ERROR_NO_DATA:
        // The pipe is being closed.
        return false;
    default:
        return false;
    }

    return true;
}

bool ChannelWin::ProcessOutgoingMessages(base::MessageLoopForIO::IOContext* context,
                                         DWORD bytes_written) 
{
    if (output_state_.is_pending) 
    {
        output_state_.is_pending = false;
        if (!context || bytes_written == 0) 
        {
            DWORD err = GetLastError();
            return false;
        }

        // 在这里删除上一次已经发送的 element
        OutputElement* element = output_queue_.front();
        output_queue_.pop();
        delete element;
    }

    if (output_queue_.empty())
        return true;

    if (!pipe_.IsValid())
        return false;

    // Write to pipe...
    OutputElement* element = output_queue_.front();

    BOOL ok = WriteFile(pipe_.Get(),
                        element->data(),
                        static_cast<uint32_t>(element->size()),
                        NULL,
                        &output_state_.context.overlapped);
    if (!ok) 
    {
        DWORD write_error = GetLastError();
        if (write_error == ERROR_IO_PENDING) 
        {
            output_state_.is_pending = true;
            return true;
        }
        return false;
    }

    output_state_.is_pending = true;
    return true;
}

void ChannelWin::OnIOCompleted(base::MessageLoopForIO::IOContext* context,
                               DWORD bytes_transfered,
                               DWORD error) 
{
    // 这里的context是 ConnectNamedPipe,ReadFile，WriteFile调用时传递进去的重叠结构体

    bool ok = true;
    if (context == &input_state_.context)
    {
        if (waiting_connect_) 
        {
            if (!ProcessConnection())
                return;

            // We may have some messages queued up to send...
            if (!output_queue_.empty() && !output_state_.is_pending)
                ProcessOutgoingMessages(NULL, 0);

            if (input_state_.is_pending)
                return;
        }

        // Process the new data.
        if (input_state_.is_pending) 
        {
            // This is the normal case for everything except the initialization step.
            input_state_.is_pending = false;
            if (!bytes_transfered) 
            {
                ok = false;
            } 
            else if (pipe_.IsValid()) 
            {
                ok = (AsyncReadComplete(bytes_transfered) != DISPATCH_ERROR);
            }
        } 

        // Request more data.
        if (ok)
            ok = (ProcessIncomingMessages() != DISPATCH_ERROR);
    } 
    else 
    {
        ok = ProcessOutgoingMessages(context, bytes_transfered);
    }

    if (!ok && pipe_.IsValid()) 
    {
        // We don't want to re-enter Close().
        Close();
        listener()->OnChannelError();
    }
}

//------------------------------------------------------------------------------
// Channel's methods

// static
std::unique_ptr<Channel> Channel::Create(const IPC::ChannelHandle& channel_handle,
                                         Mode mode,
                                         Listener* listener) 
{
    return std::unique_ptr<Channel>(new ChannelWin(channel_handle, mode, listener));
}


// static
bool Channel::IsNamedServerInitialized(const std::string& channel_id) 
{
    return ChannelWin::IsNamedServerInitialized(channel_id);
}


}  // namespace IPC
