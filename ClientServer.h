#pragma once
#include "Graphics.h"

class SeaBattle;

class ClientServer
{
public:

	enum class DOIT : unsigned char
	{
		PUSHMAP,
		STARTGAME,
		STOPGAME,
		END,
		HIT,
		CONNECTIONERROR,
		CONNECTED,
		SHIPADDITION,
		WAITRIVAL,
		MYMOVE,
		RIVALMOVE
	};

	ClientServer() = delete;
	explicit ClientServer(Graphics& g, SeaBattle& c) : _graphics(g), _client(c) { }
	virtual ~ClientServer() = default;
	ClientServer(const ClientServer&) = delete;
	ClientServer(ClientServer&&) = delete;
	ClientServer& operator=(const ClientServer&) = delete;
	ClientServer& operator=(ClientServer&&) = delete;

	void Send(const Packet& packet);
	void Disconnect();
	[[nodiscard]] bool StartServer(int port);
	[[nodiscard]] bool StartClient(const QString& ip, int port);

	[[nodiscard]] DOIT GetGameState() const noexcept
	{
		return _currentState;
	}

protected:

	bool Connect();

private:

	Graphics& _graphics;
	SeaBattle& _client;
	DOIT _currentState = DOIT::STARTGAME;
};
