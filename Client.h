#pragma once
#include "NetworkInterface.h"
#include "QTcpSocket"

class Client : public NetworkInterface
{
	Q_OBJECT

public:

	explicit Client(Graphics& g, QObject* parent, NetworkInterface** r);
	Client() = delete;
	virtual ~Client() = default;
	Client(const Client&) = delete;
	Client(Client&&) = delete;
	Client& operator=(const Client&) = delete;
	Client& operator=(Client&&) = delete;

	void Close() override;
	void Connect(const QString& ip, quint16 port);

protected:

	bool _closed = true;

	void Send(const Packet& packet) override
	{
		packet.Send(*_tcpSocket);
	}

private:

	QTcpSocket* const _tcpSocket = new QTcpSocket{ this };
	void IncomingProc(Packet packet);

signals:
	void NeedDelete();

private slots:

	void SlotDeleteMe()
	{
		_closed = true;
		emit NeedDelete();
	}

	void SlotReadyRead()
	{
		IncomingProc(Packet(*_tcpSocket));
	}

	void SlotError(const QAbstractSocket::SocketError err)
	{
		if (err == QAbstractSocket::RemoteHostClosedError)
			IncomingProc(Packet(Packet::STATE::DISCONNECTED));
		else
			IncomingProc(Packet(GetErrorDescr(err)));
	}

	void SlotConnected()
	{
		_closed = false;
		_currentState = STATE::PUSHMAP;
		IncomingProc(Packet());
	}
};
