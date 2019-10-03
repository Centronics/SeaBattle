#pragma once
#include "QTcpServer"
#include "Packet.h"

class ServerThread : public QTcpServer
{
	Q_OBJECT

public:

	ServerThread(QObject* creator) : _creator(creator)
	{
		connect(this, SIGNAL(newConnection()), SLOT(SlotNewConnection()), Qt::DirectConnection);
	}

	ServerThread(const ServerThread&) = delete;
	ServerThread(ServerThread&&) = delete;
	ServerThread& operator=(const ServerThread&) = delete;
	ServerThread& operator=(ServerThread&&) = delete;
	virtual ~ServerThread() = default;//ÂÛÏÎËÍßÅÒÑß ËÈ?

private:

	QObject* _creator = nullptr;
	std::optional<Packet> _sendMe;
	QTcpSocket* _socket = nullptr;

signals:

	void SigNewConnection();
	void SigRead(QTcpSocket*, std::optional<Packet>*);
	void SigError(std::optional<QAbstractSocket::SocketError>);

private slots:

	void SlotError(const QAbstractSocket::SocketError err)
	{
		emit SigError(err);
	}

	void SlotDisconnected()
	{
		emit SigError(std::nullopt);
	}

	void SlotReadClient()
	{
		emit SigRead(_socket, &_sendMe);
		if (_sendMe)
			_sendMe->Send(*qobject_cast<QTcpSocket*>(sender()));
	}

	void SlotSend(const Packet packet)
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
		connect(pClientSocket, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(SlotError(QAbstractSocket::SocketError)), Qt::DirectConnection);//ÒÅÑÒÈĞÎÂÀÒÜ ÂÇÀÈÌÎÁËÎÊÈĞÎÂÊÈ Ñ ÃËÀÂÍÛÌ ÏÎÒÎÊÎÌ
		connect(pClientSocket, SIGNAL(disconnected()), SLOT(SlotDisconnected()), Qt::DirectConnection);

		_socket = pClientSocket;
		emit SigNewConnection();
	}
};