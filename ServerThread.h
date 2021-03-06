#pragma once
#include "Packet.h"
#include "NetworkInterface.h"
#include "QTcpServer"

class ServerThread : public QTcpServer
{
	Q_OBJECT

public:

	explicit ServerThread(NetworkInterface* creator) : _creator(creator)
	{
		Q_UNUSED(connect(this, SIGNAL(newConnection()), SLOT(SlotNewConnection()), Qt::DirectConnection));
	}

	ServerThread(const ServerThread&) = delete;
	ServerThread(ServerThread&&) = delete;
	ServerThread& operator=(const ServerThread&) = delete;
	ServerThread& operator=(ServerThread&&) = delete;
	virtual ~ServerThread() = default;

private:

	NetworkInterface* _creator = nullptr;
	QTcpSocket* _socket = nullptr;
	bool _raiseEvents = true;

signals:

	void SigNewConnection();
	void SigRead(QTcpSocket*, std::variant<Packet, NetworkInterface::STATUS>*);
	void SigError(std::optional<QAbstractSocket::SocketError>);

private slots:

	void SlotDisconnected()
	{
		if (!_creator || !_raiseEvents)
			return;
		emit SigError(std::nullopt);
		_creator->Close();
		_creator = nullptr;
	}

	void SlotError(const QAbstractSocket::SocketError err)
	{
		if (!_creator || !_raiseEvents)
			return;
		emit SigError(err);
		_creator->Close();
		_creator = nullptr;
	}

	void SlotReadClient()
	{
		if (!_raiseEvents)
			return;
		std::variant<Packet, NetworkInterface::STATUS> sendMe;
		emit SigRead(_socket, &sendMe);
		if (_creator)
			_creator->EventHandler(sendMe, *qobject_cast<QTcpSocket*>(sender()));
	}

	void SlotSend(const Packet packet) const
	{
		QTcpSocket* s = _socket;
		if (s)
			packet.Send(*s);
	}

	void SlotNewConnection()
	{
		if (!_raiseEvents)
			return;
		QTcpSocket* pClientSocket = nextPendingConnection();

		if (_socket)
		{
			Q_UNUSED(connect(pClientSocket, SIGNAL(disconnected()), pClientSocket, SLOT(deleteLater()), Qt::DirectConnection));
			const Packet p(Packet::STATE::BUSY);
			p.Send(*pClientSocket);
			pClientSocket->close();
			return;
		}

		Q_UNUSED(connect(pClientSocket, SIGNAL(readyRead()), SLOT(SlotReadClient()), Qt::DirectConnection));
		Q_UNUSED(connect(pClientSocket, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(SlotError(QAbstractSocket::SocketError)), Qt::DirectConnection));
		Q_UNUSED(connect(pClientSocket, SIGNAL(disconnected()), SLOT(SlotDisconnected()), Qt::DirectConnection));

		_socket = pClientSocket;
		emit SigNewConnection();
	}

public slots:

	void SlotClose()
	{
		_raiseEvents = false;
		close();
		if (_socket)
			_socket->close();
	}
};