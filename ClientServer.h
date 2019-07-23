#pragma once
#include "Graphics.h"
#include "Packet.h"

class SeaBattle;

class ClientServer
{
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

	[[nodiscard]] DOIT GetGameState() const noexcept
	{
		return _currentState;
	}

protected:

	bool Connect();
	void Send();
	void Receive();

private:

	Graphics& _graphics;
	SeaBattle& _client;
	DOIT _currentState = DOIT::STARTGAME;
	Packet _packet;
};
