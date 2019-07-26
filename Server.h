#pragma once
#include "NetworkInterface.h"
#include "QTcpServer"

class SeaBattle;

class Server : protected NetworkInterface
{
public:

	explicit Server(Graphics& g, SeaBattle& c, QObject* parent);
	Server() = delete;
	virtual ~Server() = default;
	Server(const Server&) = delete;
	Server(Server&&) = delete;
	Server& operator=(const Server&) = delete;
	Server& operator=(Server&&) = delete;

	void SendHit(quint8 coord) override;
	bool Listen(int port);

	[[nodiscard]] QString GetErrorString() const override
	{
		return _server.errorString();
	}

private slots:

	void SlotNewConnection();
	void SlotReadClient();

private:

	QTcpServer _server{ this };
	[[nodiscard]] std::optional<Packet> GetFromQueue();
	void SendToClient(QTcpSocket* socket) const;
	void Send();
	void Receive();
};
