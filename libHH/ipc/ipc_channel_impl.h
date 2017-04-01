#ifndef IPC_CHANNEL_IMPL_H
#define IPC_CHANNEL_IMPL_H

#include <QObject>
#include <QMap>

#include "ipc_channel.h"
#include "ipc_channel_reader.h"

class QLocalServer;
class QLocalSocket;

namespace IPC {

	class ChannelServer : public QObject, 
						  public Channel, 
						  public ChannelReader
	{
		Q_OBJECT

	public:
		ChannelServer(const QString &name, Listener* listener, QObject *parent = 0);
		~ChannelServer();

	public:
		// Channel implementation
		virtual bool Connect();
		virtual void Close();
		virtual bool Send(Message* msg);

	private slots:
		void OnNewConnection();
		void OnReadyRead();
		void OnClientDisconnected();

	private:
		qint64 FindProcessId(QLocalSocket* socket);
		bool SendAll(Message* msg);
		bool SendOne(qint64 pid, Message* msg);

	private:
		QLocalServer* server_;
		QMap<qint64, QLocalSocket*> clients_;
	};



	class ChannelClient : public QObject, 
						  public Channel, 
						  public ChannelReader
	{
		Q_OBJECT

	public:
		ChannelClient(const QString &name, Listener* listener, QObject *parent = 0);
		~ChannelClient();

	public:
		// Channel implementation
		virtual bool Connect();
		virtual void Close();
		virtual bool Send(Message* msg);

	private slots:
		void OnReadyRead();
		void OnConnected();

	private:
		QLocalSocket* client_;

	};

}


#endif // IPC_CHANNEL_IMPL_H
