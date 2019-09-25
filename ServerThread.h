#pragma once
#include "QTcpServer"
#include "Packet.h"

class ServerThread : public QTcpServer
{
	Q_OBJECT

public:

	ServerThread() = default;
	ServerThread(const ServerThread&) = delete;
	ServerThread(ServerThread&&) = delete;
	ServerThread& operator=(const ServerThread&) = delete;
	ServerThread& operator=(ServerThread&&) = delete;
	virtual ~ServerThread() = default;

private:

	QTcpSocket* _socket = nullptr;

signals:

	void SigNewConnection();
	void SigRead(QTcpSocket*);
	void SigClose();
	void SigError(QAbstractSocket::SocketError);

private slots:

	void SlotError(const QAbstractSocket::SocketError err)
	{
		emit SigError(err);
	}

	void SlotReadClient()
	{
		emit SigRead(_socket);
	}

	void SlotSend(const Packet packet)
	{
		QTcpSocket* s = _socket;
		if (s)
			packet.Send(*s);
	}

	void SlotClose()
	{
		emit SigClose();
	}

	void SlotNewConnection()
	{
		QTcpSocket* pClientSocket = nextPendingConnection();
		connect(pClientSocket, SIGNAL(disconnected()), pClientSocket, SLOT(deleteLater()), Qt::DirectConnection);
		if (_socket)
		{
			const Packet p(Packet::STATE::BUSY);
			p.Send(*pClientSocket);
			pClientSocket->close();
			return;
		}

		connect(pClientSocket, SIGNAL(disconnected()), SLOT(SlotClose()), Qt::DirectConnection);
		connect(pClientSocket, SIGNAL(readyRead()), SLOT(SlotReadClient()), Qt::DirectConnection);
		connect(pClientSocket, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(SlotError(QAbstractSocket::SocketError)), Qt::DirectConnection);

		_socket = pClientSocket;
		emit SigNewConnection();
	}
};