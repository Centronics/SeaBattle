#pragma once
#include "Packet.h"

class ClientThread : public QTcpSocket
{
	Q_OBJECT

public:

	explicit ClientThread(QObject* creator) : _creator(creator)
	{
		connect(this, SIGNAL(disconnected()), SLOT(SlotDisconnected()), Qt::DirectConnection);
		connect(this, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(SlotError(QAbstractSocket::SocketError)), Qt::DirectConnection);
	}

	ClientThread(const ClientThread&) = delete;
	ClientThread(ClientThread&&) = delete;
	ClientThread& operator=(const ClientThread&) = delete;
	ClientThread& operator=(ClientThread&&) = delete;
	virtual ~ClientThread() = default;

private:

	QObject* const _creator = nullptr;

private slots:

	void SlotSend(const Packet packet)
	{
		packet.Send(*this);
	}

	void SlotDisconnected()
	{
		emit SigError(std::nullopt);
		if (_creator)
			_creator->deleteLater();
	}

	void SlotError(QAbstractSocket::SocketError err)
	{
		emit SigError(err);
		if (_creator)
			_creator->deleteLater();
	}

signals:

	void SigError(std::optional<QAbstractSocket::SocketError>);
};
