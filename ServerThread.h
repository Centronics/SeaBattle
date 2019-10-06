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
		connect(this, SIGNAL(newConnection()), SLOT(SlotNewConnection()), Qt::DirectConnection);
	}

	ServerThread(const ServerThread&) = delete;
	ServerThread(ServerThread&&) = delete;
	ServerThread& operator=(const ServerThread&) = delete;
	ServerThread& operator=(ServerThread&&) = delete;
	virtual ~ServerThread() = default;

private:

	NetworkInterface* _creator = nullptr;
	std::variant<Packet, NetworkInterface::STATUS> _sendMe;
	QTcpSocket* _socket = nullptr;

signals:

	void SigNewConnection();
	void SigRead(QTcpSocket*, std::variant<Packet, NetworkInterface::STATUS>*);
	void SigError(std::optional<QAbstractSocket::SocketError>);

private slots:

	void SlotDisconnected()
	{
		if (!_creator)
			return;
		emit SigError(std::nullopt);
		_creator->Close();
		_creator = nullptr;
	}

	void SlotError(const QAbstractSocket::SocketError err)
	{
		if (!_creator)
			return;
		emit SigError(err);
		_creator->Close();
		_creator = nullptr;
	}

	void SlotReadClient()
	{
		emit SigRead(_socket, &_sendMe);
		if (_creator)
			_creator->EventHandler(_sendMe, *qobject_cast<QTcpSocket*>(sender()));
	}

	void SlotSend(const Packet packet) const
	{
		QTcpSocket* s = _socket;
		if (s)
			packet.Send(*s);
	}

	void SlotNewConnection()
	{
		QTcpSocket* pClientSocket = nextPendingConnection();

		if (_socket)
		{
			connect(pClientSocket, SIGNAL(disconnected()), pClientSocket, SLOT(deleteLater()), Qt::DirectConnection);
			const Packet p(Packet::STATE::BUSY);
			p.Send(*pClientSocket);
			pClientSocket->close();
			return;
		}

		connect(pClientSocket, SIGNAL(readyRead()), SLOT(SlotReadClient()), Qt::DirectConnection);
		connect(pClientSocket, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(SlotError(QAbstractSocket::SocketError)), Qt::DirectConnection);
		connect(pClientSocket, SIGNAL(disconnected()), SLOT(SlotDisconnected()), Qt::DirectConnection);

		_socket = pClientSocket;
		emit SigNewConnection();
	}
};