#pragma once
#include "NetworkInterface.h"
#include "QTcpServer"
#include "QTcpSocket"

class SeaBattle;

class Server : public NetworkInterface
{
	Q_OBJECT

public:

	explicit Server(Graphics& g, SeaBattle& c, QObject* parent) : NetworkInterface(g, c, parent)
	{
		connect(&_server, SIGNAL(newConnection()), SLOT(SlotNewConnection()));
		_currentState = STATE::WAITMAP;
	}

	Server() = delete;
	~Server() = default;
	Server(const Server&) = delete;
	Server(Server&&) = delete;
	Server& operator=(const Server&) = delete;
	Server& operator=(Server&&) = delete;

	void SendHit(quint8 coord) override;
	void Close() override;
	void Listen(quint16 port);

protected:

	void SendToClient(Packet packet) const;

private:

	QTcpServer _server{ this };
	QTcpSocket* _socket = nullptr;

	void SendAnswerToClient(Packet packet);
	void SocketClose();

private slots:

	void SlotNewConnection();
	void SlotReadClient();
};
