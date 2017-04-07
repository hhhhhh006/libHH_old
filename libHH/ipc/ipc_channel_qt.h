#ifndef IPC_CHANNEL_IMPL_H
#define IPC_CHANNEL_IMPL_H

#include <QObject>
#include <QMap>

#include "ipc/ipc_channel.h"
#include "ipc/ipc_channel_reader.h"

class QLocalServer;
class QLocalSocket;

namespace IPC {

class ChannelServer : public QObject, 
					  public Channel, 
                      public internal::ChannelReader
{
    Q_OBJECT

public:
    ChannelServer(const ChannelHandle& channel_handle, Listener* listener, QObject *parent = 0);
    ~ChannelServer();

public:
    // Channel implementation
    virtual bool Connect();
    virtual void Close();
    virtual bool Send(Message* msg);

    virtual base::ProcessId GetPeerPID() const { return 1; }
    virtual base::ProcessId GetSelfPID() const { return 1; }
    virtual ReadState ReadData(char* buffer, int buffer_len, int* bytes_read) { return READ_FAILED; }
    virtual base::ProcessId GetSenderPID() { return 1; }
    virtual void HandleInternalMessage(const Message& msg) {}

private slots:
    void OnNewConnection();
    void OnReadyRead();
    void OnClientDisconnected();

private:
    qint64 FindProcessId(QLocalSocket* socket);
    bool SendAll(Message* msg);
    bool SendOne(qint64 pid, Message* msg);
    void SendHelloMessage(QLocalSocket *socket);

private:
    QLocalServer* server_;
    QMap<qint64, QLocalSocket*> clients_;
};



class ChannelClient : public QObject, 
					  public Channel, 
                      public internal::ChannelReader
{
    Q_OBJECT

public:
    ChannelClient(const ChannelHandle& channel_handle, Listener* listener, QObject *parent = 0);
    ~ChannelClient();

public:
    // Channel implementation
    virtual bool Connect();
    virtual void Close();
    virtual bool Send(Message* msg);

    virtual base::ProcessId GetPeerPID() const { return 1; }
    virtual base::ProcessId GetSelfPID() const { return 1; }
    virtual ReadState ReadData(char* buffer, int buffer_len, int* bytes_read) { return READ_FAILED; }
    virtual base::ProcessId GetSenderPID() { return 1; }
    virtual void HandleInternalMessage(const Message& msg) {}

private slots:
    void OnReadyRead();
    void OnConnected();

private:
    QLocalSocket* client_;

};

}  // namespace IPC


#endif // IPC_CHANNEL_IMPL_H
