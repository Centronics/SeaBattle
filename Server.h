#pragma once
#include "NetworkInterface.h"
#include "QTcpServer"
#include "QTcpSocket"

class SeaBattle;

class Server : public NetworkInterface
{
	Q_OBJECT

public:

	explicit Server(Graphics& g, SeaBattle& c, QObject* parent);
	Server() = delete;
	virtual ~Server() = default;
	Server(const Server&) = delete;
	Server(Server&&) = delete;
	Server& operator=(const Server&) = delete;
	Server& operator=(Server&&) = delete;

	void SendHit(quint8 coord) override;
	void Listen(quint16 port);

private:

	QTcpServer _server{ this };
	std::unique_ptr<QTcpSocket> _socket;

	void SendToClient(const Packet& packet) const;
	void IncomingProc(Packet packet);

private slots:

	void SlotNewConnection();
	void SlotReadClient();
	void SlotError(QAbstractSocket::SocketError err);
};
