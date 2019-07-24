#pragma once
#include "Graphics.h"
#include "Packet.h"
#include "QTcpServer"
#include "QTcpSocket"
#include <queue>

class SeaBattle;

class ClientServer : QObject
{
	Q_OBJECT

public:

	ClientServer() = delete;
	explicit ClientServer(Graphics& g, SeaBattle& c) : _graphics(g), _client(c) { }
	virtual ~ClientServer() = default;
	ClientServer(const ClientServer&) = delete;
	ClientServer(ClientServer&&) = delete;
	ClientServer& operator=(const ClientServer&) = delete;
	ClientServer& operator=(ClientServer&&) = delete;
	void SendHit(quint8 coord);
	void Disconnect();
	[[nodiscard]] bool StartServer(int port);
	[[nodiscard]] bool StartClient(const QString& ip, int port);
	[[nodiscard]] bool Listen(int port);
	[[nodiscard]] Packet GetFromQueue() const;
	void Send(const Packet& packet);

	[[nodiscard]] QString GetErrorString() const
	{
		return _server.errorString();
	}

	[[nodiscard]] DOIT GetGameState() const noexcept
	{
		return _currentState;
	}

public slots:
	void SlotNewConnection();
	void SlotReadClient();

protected:

	mutable std::recursive_mutex _lock;
	mutable std::queue<Packet> _requests;

	bool Connect();
	void Send();
	void Receive();
	void AddToQueue(const Packet& packet);

private:

	Graphics& _graphics;
	SeaBattle& _client;
	DOIT _currentState = DOIT::STARTGAME;
	Packet _packet;
	QTcpServer _server{ this };

	void SendToClient(QTcpSocket* socket);
};
