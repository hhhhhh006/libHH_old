#include "ipc/ipc_channel_qt.h"

#include <QLocalServer>
#include <QLocalSocket>
#include <QCoreApplication>

#include "base/memory/ptr_util.h"
#include "ipc/ipc_message.h"
#include "ipc/ipc_listener.h"


namespace IPC{

//channel server
ChannelServer::ChannelServer(const ChannelHandle& channel_handle, Listener* listener, QObject *parent)	
    : QObject(parent),
      ChannelReader(listener)
{
	server_ = new QLocalServer(this);
	server_->listen(QString::fromStdString(channel_handle));
		
	connect(server_, SIGNAL(newConnection()), this, SLOT(OnNewConnection()));
}

ChannelServer::~ChannelServer()
{

}

bool ChannelServer::Connect()
{
    return true;
}

void ChannelServer::Close()
{
    server_->close();
}

bool ChannelServer::Send(Message* msg)
{
    if (msg->routing_id() == MSG_ROUTING_CONTROL)
    {
        return SendAll(msg);
    }
    else
    {
        return SendOne(msg->routing_id(), msg);
    }
    return false;
}

void ChannelServer::OnNewConnection()
{
    QLocalSocket *socket = server_->nextPendingConnection();
    connect(socket, SIGNAL(readyRead()), this, SLOT(OnReadyRead()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(OnClientDisconnected()));
    SendHelloMessage(socket);
}

void ChannelServer::OnReadyRead()
{
    QLocalSocket *socket = qobject_cast<QLocalSocket *>(sender());
    QByteArray data = socket->readAll();
    InputData(data.data(), data.size());

    // client must send a 'hello message type' for confirming the connection
    int32_t pid = FindProcessId(socket);
    if (pid == 0)
    {
        Message msg;
        bool rs = GetHelloMessage(&msg);
        if (rs)
        {
            base::PickleIterator iterator(msg);
            iterator.ReadInt32(&pid);
            if (pid > 0)
            {
                clients_.insert(pid, socket);

                listener()->OnChannelConnected(pid);
            }
        }
    }
}

void ChannelServer::OnClientDisconnected()
{
    QLocalSocket *socket = qobject_cast<QLocalSocket *>(sender());
    qint64 pid = FindProcessId(socket);
    clients_.remove(pid);
}

bool ChannelServer::SendAll(Message* msg)
{
    QMap<qint64, QLocalSocket*>::iterator it = clients_.begin();
    while (it != clients_.end())
    {
        QLocalSocket *socket = *it;
        socket->write(reinterpret_cast<const char*>(msg->data()), msg->size());

        it++;
    }
    return true;
}

bool ChannelServer::SendOne(qint64 pid, Message* msg)
{
    QMap<qint64, QLocalSocket*>::iterator it = clients_.find(pid);
    if (it == clients_.end())
        return false;

    QLocalSocket *socket = *it;
    socket->write(reinterpret_cast<const char*>(msg->data()), msg->size());
    return true;
}

void ChannelServer::SendHelloMessage(QLocalSocket *socket)
{
    qint64 pid = QCoreApplication::applicationPid();
    IPC::Message msg(MSG_ROUTING_NONE, IPC::Channel::HELLO_MESSAGE_TYPE);
    msg.WriteInt32(pid);
    socket->write(reinterpret_cast<const char*>(msg.data()), msg.size());
}

qint64 ChannelServer::FindProcessId(QLocalSocket* socket)
{
    QMap<qint64, QLocalSocket*>::iterator it = clients_.begin();
    while (it != clients_.end())
    {
        if (socket == (*it))
        {
            return it.key();
        }
        it++;
    }
    return 0;
}


//channel client
ChannelClient::ChannelClient(const ChannelHandle& channel_handle, Listener* listener, QObject *parent /*= 0*/)
    :QObject(parent),
    ChannelReader(listener)
{
    client_ = new QLocalSocket(this);
    client_->setServerName(QString::fromStdString(channel_handle));

    connect(client_, SIGNAL(connected()), this, SLOT(OnConnected()));
    connect(client_, SIGNAL(readyRead()), this, SLOT(OnReadyRead()));
}

ChannelClient::~ChannelClient()
{

}

bool ChannelClient::Connect()
{
    client_->connectToServer();
    return true;
}

void ChannelClient::Close()
{
    client_->disconnectFromServer();
}

bool ChannelClient::Send(Message* msg)
{
    client_->write(reinterpret_cast<const char*>(msg->data()), msg->size());
    return true;
}

void ChannelClient::OnReadyRead()
{
    QByteArray data = client_->readAll();
    InputData(data.data(), data.size());

    int32_t pid;
    Message msg;
    bool rs = GetHelloMessage(&msg);
    if (rs)
    {
        base::PickleIterator iterator(msg);
        iterator.ReadInt32(&pid);
        if (pid > 0)
        {
            listener()->OnChannelConnected(pid);
        }
    }
}

void ChannelClient::OnConnected()
{
    qint64 pid = QCoreApplication::applicationPid();
    IPC::Message msg(MSG_ROUTING_NONE, IPC::Channel::HELLO_MESSAGE_TYPE);
    msg.WriteInt32(pid);
    Send(&msg);
}


#ifdef OS_WIN

// Channel's static methods
std::unique_ptr<Channel> Channel::Create(const ChannelHandle& channel_handle, Mode mode, Listener* listener)
{
    if (mode & MODE_SERVER)
    {
        return base::WrapUnique(new ChannelServer(channel_handle, listener));
    }
    else if (mode & MODE_CLIENT)
    {
        return base::WrapUnique(new ChannelClient(channel_handle, listener));
    }
    return base::WrapUnique<Channel>(nullptr);
}

#endif // OS_WIN

}