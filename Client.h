#pragma once
#include "NetworkInterface.h"
#include "QTcpSocket"

class SeaBattle;

class Client : public NetworkInterface
{
	Q_OBJECT

public:

	explicit Client(Graphics& g, SeaBattle& c, QObject* parent);
	Client() = delete;
	virtual ~Client() = default;
	Client(const Client&) = delete;
	Client(Client&&) = delete;
	Client& operator=(const Client&) = delete;
	Client& operator=(Client&&) = delete;

	void SendHit() override
	{
		SendToServer(CreateHitPacket());
	}

	void Connect(const QString& ip, const quint16 port)
	{
		_tcpSocket.close();
		_tcpSocket.connectToHost(ip, port, QIODevice::ReadWrite, QAbstractSocket::NetworkLayerProtocol::IPv4Protocol);
	}

private:

	QTcpSocket _tcpSocket{ this };

	void SendToServer(const Packet& packet)
	{
		if (packet)
			_tcpSocket.write(GetBytes(packet));
	}

	void Close()
	{
		_currentState = STATE::PUSHMAP;
		_tcpSocket.close();
	}

	void IncomingProc(Packet packet);

private slots:

	void SlotReadyRead()
	{
		IncomingProc(Packet(_tcpSocket));
	}

	void SlotError(const QAbstractSocket::SocketError err)
	{
		IncomingProc(Packet(GetErrorDescr(err)));
	}

	void SlotConnected()
	{
		_currentState = STATE::PUSHMAP;
		IncomingProc(Packet());
	}
};
