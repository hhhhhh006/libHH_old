#include "ipc_channel_impl.h"
#include "ipc_message.h"
#include "ipc_listener.h"

#include <QLocalServer>
#include <QLocalSocket>

namespace IPC{

	//channel server
	ChannelServer::ChannelServer(const QString &name, Listener* listener, QObject *parent)
		: QObject(parent),
		  ChannelReader(listener)
	{
		server_ = new QLocalServer(this);
		server_->listen(name);
		
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
		listener()->OnChannelDisconnected(pid);
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
	ChannelClient::ChannelClient(const QString &name, Listener* listener, QObject *parent /*= 0*/)
		:QObject(parent),
		 ChannelReader(listener)
	{
		client_ = new QLocalSocket(this);
		client_->setServerName(name);

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
	}

	void ChannelClient::OnConnected()
	{
		listener()->OnChannelConnected(0);
	}



	// Channel's static methods
	Channel* Channel::Create(const QString& name, Mode mode, Listener* listener)
	{
		if (mode & MODE_SERVER)
		{
			return new ChannelServer(name, listener);
		}
		else if (mode & MODE_CLIENT)
		{
			return new ChannelClient(name, listener);
		}
		return nullptr;
	}

}