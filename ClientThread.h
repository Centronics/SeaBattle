#pragma once
#include "Packet.h"
#include "NetworkInterface.h"

class ClientThread : public QTcpSocket
{
	Q_OBJECT

public:

	explicit ClientThread(NetworkInterface* creator) : _creator(creator)
	{
		Q_UNUSED(connect(this, SIGNAL(connected()), SLOT(SlotConnected()), Qt::DirectConnection));
		Q_UNUSED(connect(this, SIGNAL(disconnected()), SLOT(SlotDisconnected()), Qt::DirectConnection));
		Q_UNUSED(connect(this, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(SlotError(QAbstractSocket::SocketError)), Qt::DirectConnection));
	}

	ClientThread(const ClientThread&) = delete;
	ClientThread(ClientThread&&) = delete;
	ClientThread& operator=(const ClientThread&) = delete;
	ClientThread& operator=(ClientThread&&) = delete;
	virtual ~ClientThread() = default;

private:

	NetworkInterface* _creator = nullptr;
	bool _raiseEvents = true;

private slots:

	void SlotDisconnected()
	{
		if (!_creator || !_raiseEvents)
			return;
		emit SigError(std::nullopt);
		_creator->Close();
		_creator = nullptr;
	}

	void SlotError(QAbstractSocket::SocketError err)
	{
		if (!_creator || !_raiseEvents)
			return;
		emit SigError(err);
		_creator->Close();
		_creator = nullptr;
	}

	void SlotReadServer()
	{
		std::variant<Packet, NetworkInterface::STATUS> sendMe;
		emit SigReadServer(&sendMe);
		if (_creator)
			_creator->EventHandler(sendMe, *qobject_cast<QTcpSocket*>(sender()));
	}

	void SlotSend(const Packet packet)
	{
		packet.Send(*this);
	}

	void SlotConnected()
	{
		if (!_raiseEvents)
			return;
		std::variant<Packet, NetworkInterface::STATUS> sendMe;
		emit SigConnected(&sendMe);
		if (_creator)
			_creator->EventHandler(sendMe, *qobject_cast<QTcpSocket*>(sender()));
	}

signals:

	void SigError(std::optional<QAbstractSocket::SocketError>);
	void SigConnected(std::variant<Packet, NetworkInterface::STATUS>*);
	void SigReadServer(std::variant<Packet, NetworkInterface::STATUS>*);

public slots:

	void SlotClose()
	{
		_raiseEvents = false;
		close();
	}
};