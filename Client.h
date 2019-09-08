#pragma once
#include "NetworkInterface.h"
#include "QTcpSocket"

class Client : public NetworkInterface
{
	Q_OBJECT

public:

	explicit Client(Graphics& g, QObject* parent);
	Client() = delete;
	virtual ~Client() = default;
	Client(const Client&) = delete;
	Client(Client&&) = delete;
	Client& operator=(const Client&) = delete;
	Client& operator=(Client&&) = delete;

	void Connect(const QString& ip, const quint16 port)
	{
		_tcpSocket.close();
		_tcpSocket.connectToHost(ip, port, QIODevice::ReadWrite, QAbstractSocket::NetworkLayerProtocol::IPv4Protocol);
	}

protected:

	void Send(const Packet& packet) override
	{
		packet.Send(_tcpSocket);
	}

private:

	QTcpSocket _tcpSocket{ this };
	void IncomingProc(Packet packet);

private slots:

	void SlotReadyRead()
	{
		IncomingProc(Packet(_tcpSocket));
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
		_currentState = STATE::PUSHMAP;
		IncomingProc(Packet());
	}
};
