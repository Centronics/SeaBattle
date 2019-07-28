#pragma once
#include "NetworkInterface.h"
#include "QTcpServer"
#include "QTcpSocket"

class SeaBattle;

class Server : protected NetworkInterface
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
	void Listen(int port);

	[[nodiscard]] QString GetErrorString() const override
	{
		return _server.errorString();
	}

private slots:

	void SlotNewConnection();
	void SlotReadClient();

private:

	QTcpServer _server{ this };
	QTcpSocket* _socket = nullptr;

	[[nodiscard]] std::optional<Packet> GetFromQueue();
	void SendToClient() const;
	void Send();
	void Receive();
};
