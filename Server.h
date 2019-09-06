#pragma once
#include "NetworkInterface.h"
#include "QTcpServer"
#include "QTcpSocket"

class Server : public NetworkInterface
{
	Q_OBJECT

public:

	explicit Server(Graphics& g, QObject* parent) : NetworkInterface(g, parent)
	{
		connect(&_server, SIGNAL(newConnection()), SLOT(SlotNewConnection()));
		connect(&_server, SIGNAL(acceptError(QAbstractSocket::SocketError)), SLOT(SlotError(QAbstractSocket::SocketError)));
	}

	Server() = delete;
	virtual ~Server() = default;
	Server(const Server&) = delete;
	Server(Server&&) = delete;
	Server& operator=(const Server&) = delete;
	Server& operator=(Server&&) = delete;

	void SendHit() override
	{
		SendToClient(CreateHitPacket());
	}

	void Listen(quint16 port);

private:

	QTcpServer _server{ this };
	std::unique_ptr<QTcpSocket> _socket;

	void IncomingProc(Packet packet);

	void SendToClient(const Packet& packet) const
	{
		if (_socket)
			packet.Send(*_socket);
	}

private slots:

	void SlotNewConnection();

	void SlotReadClient()
	{
		IncomingProc(Packet(*qobject_cast<QTcpSocket*>(sender())));
	}

	void SlotError(const QAbstractSocket::SocketError err)
	{
		IncomingProc(Packet(GetErrorDescr(err)));
	}
};
