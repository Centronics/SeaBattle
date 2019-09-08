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

	void Listen(quint16 port);

protected:

	void Send(const Packet& packet) override
	{
		if (_socket)
			packet.Send(*_socket);
	}

private:

	QTcpServer _server{ this };
	std::unique_ptr<QTcpSocket> _socket;

	void IncomingProc(Packet packet);

private slots:

	void SlotNewConnection();

	void SlotReadClient()
	{
		IncomingProc(Packet(*qobject_cast<QTcpSocket*>(sender())));
	}

	void SlotError(const QAbstractSocket::SocketError err)
	{
		if (err == QAbstractSocket::RemoteHostClosedError)
			IncomingProc(Packet(Packet::STATE::DISCONNECTED));
		else
			IncomingProc(Packet(GetErrorDescr(err)));
	}
};
