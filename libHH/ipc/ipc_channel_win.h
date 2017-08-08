#ifndef IPC_IPC_CHANNEL_WIN_H_
#define IPC_IPC_CHANNEL_WIN_H_

#include "ipc/ipc_channel_reader.h"

#include <stdint.h>

#include <queue>
#include <string>

#include "base/macros.h"
#include "base/memory/weak_ptr.h"
#include "base/win/scoped_handle.h"
#include "base/message_loop/message_loop.h"
#include "ipc/ipc_channel.h"


namespace IPC {

class ChannelWin : public Channel,
                   public internal::ChannelReader,
                   public base::MessageLoopForIO::IOHandler 
{
public:
    ChannelWin(const IPC::ChannelHandle& channel_handle, Mode mode,
               Listener* listener);
    ~ChannelWin();

    // Channel implementation
    bool Connect() override;
    void Close() override;
    bool Send(Message* message) override;
    base::ProcessId GetPeerPID() const override;
    base::ProcessId GetSelfPID() const override;

    static bool IsNamedServerInitialized(const std::string& channel_id);


private:
    // ChannelReader implementation.
    ReadState ReadData(char* buffer, int buffer_len, int* bytes_read) override;
    void HandleInternalMessage(const Message& msg) override;
    base::ProcessId GetSenderPID() override;

    static const std::string PipeName(const std::string& channel_id);
    bool CreatePipe(const IPC::ChannelHandle &channel_handle, Mode mode);

    bool ProcessConnection();
    bool ProcessOutgoingMessages(base::MessageLoopForIO::IOContext* context,
        DWORD bytes_written);

    // Returns |false| on channel error.
    // If |message| has brokerable attachments, those attachments are passed to
    // the AttachmentBroker (which in turn invokes Send()), so this method must
    // be re-entrant.
    // Adds |message| to |output_queue_| and calls ProcessOutgoingMessages().
    bool ProcessMessageForDelivery(Message* message);

    // Moves all messages from |prelim_queue_| to |output_queue_| by calling
    // ProcessMessageForDelivery().
    void FlushPrelimQueue();

    // MessageLoop::IOHandler implementation.
    void OnIOCompleted(base::MessageLoopForIO::IOContext* context,
        DWORD bytes_transfered,
        DWORD error) override;

private:
    struct State {
        explicit State(ChannelWin* channel);
        ~State();
        base::MessageLoopForIO::IOContext context;
        bool is_pending;
    };

    State input_state_;
    State output_state_;

    base::win::ScopedHandle pipe_;

    base::ProcessId peer_pid_;

    // Messages not yet ready to be sent are queued here. Messages removed from
    // this queue are placed in the output_queue_. The double queue is
    // unfortunate, but is necessary because messages with brokerable attachments
    // can generate multiple messages to be sent (possibly from other channels).
    // Some of these generated messages cannot be sent until |peer_pid_| has been
    // configured.
    // As soon as |peer_pid| has been configured, there is no longer any need for
    // |prelim_queue_|. All messages are flushed, and no new messages are added.
    std::queue<Message*> prelim_queue_;

    // Messages to be sent are queued here.
    std::queue<OutputElement*> output_queue_;

    // In server-mode, we have to wait for the client to connect before we
    // can begin reading.  We make use of the input_state_ when performing
    // the connect operation in overlapped mode.
    bool waiting_connect_;


    base::WeakPtrFactory<ChannelWin> weak_factory_;
    DISALLOW_COPY_AND_ASSIGN(ChannelWin);
};

}  // namespace IPC

#endif  // IPC_IPC_CHANNEL_WIN_H_
