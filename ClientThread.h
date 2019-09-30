#pragma once
#include "Packet.h"

class ClientThread : public QTcpSocket
{
	Q_OBJECT

public:

	ClientThread()
	{
		connect(this, SIGNAL(disconnected()), SLOT(SlotDisconnected()), Qt::DirectConnection);
		connect(this, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(SlotError(QAbstractSocket::SocketError)), Qt::DirectConnection);
	}

	ClientThread(const ClientThread&) = delete;
	ClientThread(ClientThread&&) = delete;
	ClientThread& operator=(const ClientThread&) = delete;
	ClientThread& operator=(ClientThread&&) = delete;
	virtual ~ClientThread() = default;

private slots:

	void SlotSend(const Packet packet)
	{
		packet.Send(*this);
	}

	void SlotDisconnected()
	{
		emit SigError(std::nullopt);
	}

	void SlotError(SocketError err)
	{
		emit SigError(err);
	}

signals:

	void SigError(std::optional<SocketError>);
};
