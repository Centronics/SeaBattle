#pragma once
#include "Packet.h"

class ClientThread : public QTcpSocket
{
	Q_OBJECT

public:

	explicit ClientThread(QObject* creator) : _creator(creator)
	{
		connect(this, SIGNAL(connected()), SLOT(SlotConnected()), Qt::DirectConnection);
		connect(this, SIGNAL(disconnected()), SLOT(SlotDisconnected()), Qt::DirectConnection);
		connect(this, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(SlotError(QAbstractSocket::SocketError)), Qt::DirectConnection);

		//connect(this, SIGNAL(SigSend(Packet)), _tcpSocket, SLOT(SlotSend(Packet)));

		//connect(_creator, SIGNAL(SigSend(Packet)), this, SLOT(SlotSend(Packet)), Qt::BlockingQueuedConnection);
	}

	ClientThread(const ClientThread&) = delete;
	ClientThread(ClientThread&&) = delete;
	ClientThread& operator=(const ClientThread&) = delete;
	ClientThread& operator=(ClientThread&&) = delete;
	virtual ~ClientThread()
	{
	}

private:

	QObject* _creator = nullptr;
	std::optional<Packet> _sendMe;

private slots:

	void SlotSend(Packet packet)
	{
		packet.Send(*this);
	}

	void SlotDisconnected()
	{
		if (!_creator)
			return;
		emit SigError(std::nullopt);
		_creator->deleteLater();
		_creator = nullptr;
	}

	void SlotError(QAbstractSocket::SocketError err)
	{
		if (!_creator)
			return;
		emit SigError(err);
		_creator->deleteLater();
		_creator = nullptr;
	}

	void SlotConnected()
	{
		emit SigConnected(&_sendMe);
		if (_sendMe)
			_sendMe->Send(*this);
	}

signals:

	void SigError(std::optional<QAbstractSocket::SocketError>);
	void SigConnected(std::optional<Packet>*);
};