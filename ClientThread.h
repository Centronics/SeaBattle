#pragma once
#include "Packet.h"
#include "NetworkInterface.h"

class ClientThread : public QTcpSocket
{
	Q_OBJECT

public:

	explicit ClientThread(NetworkInterface* creator) : _creator(creator)
	{
		connect(this, SIGNAL(connected()), SLOT(SlotConnected()), Qt::DirectConnection);
		connect(this, SIGNAL(disconnected()), SLOT(SlotDisconnected()), Qt::DirectConnection);
		connect(this, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(SlotError(QAbstractSocket::SocketError)), Qt::DirectConnection);
	}

	ClientThread(const ClientThread&) = delete;
	ClientThread(ClientThread&&) = delete;
	ClientThread& operator=(const ClientThread&) = delete;
	ClientThread& operator=(ClientThread&&) = delete;
	virtual ~ClientThread()
	{
	}

private:

	NetworkInterface* _creator = nullptr;

private slots:

	void SlotDisconnected()
	{
		if (!_creator)
			return;
		emit SigError(std::nullopt);
		_creator->Close();
		_creator = nullptr;
	}

	void SlotError(QAbstractSocket::SocketError err)
	{
		if (!_creator)
			return;
		emit SigError(err);
		_creator->Close();
		_creator = nullptr;
	}

	void SlotSend(const Packet packet)
	{
		packet.Send(*this);
	}

	void SlotConnected()
	{
		std::variant<Packet, NetworkInterface::STATUS> sendMe;
		emit SigConnected(&sendMe);
		if (_creator)
			_creator->EventHandler(sendMe, *qobject_cast<QTcpSocket*>(sender()));
	}

signals:

	void SigError(std::optional<QAbstractSocket::SocketError>);
	void SigConnected(std::variant<Packet, NetworkInterface::STATUS>*);
};