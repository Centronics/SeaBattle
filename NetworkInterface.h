#pragma once
#include "Packet.h"
#include <queue>
#include "Graphics.h"

class SeaBattle;

class NetworkInterface : public QObject
{
	Q_OBJECT

public:

	explicit NetworkInterface(Graphics& g, SeaBattle& c, QObject* parent) : QObject(parent), _graphics(g), _client(c) { }
	NetworkInterface() = delete;
	virtual ~NetworkInterface() = default;
	NetworkInterface(const NetworkInterface&) = delete;
	NetworkInterface(NetworkInterface&&) = delete;
	NetworkInterface& operator=(const NetworkInterface&) = delete;
	NetworkInterface& operator=(NetworkInterface&&) = delete;

	virtual void SendHit(quint8 coord) = 0;

	[[nodiscard]] DOIT GetGameState() const noexcept
	{
		return _currentState;
	}

protected:

	mutable std::recursive_mutex _lock;
	std::queue<Packet> _requests;
	DOIT _currentState = DOIT::STARTGAME;
	Packet _packet;
	Graphics& _graphics;
	SeaBattle& _client;

signals:

	void Connected(bool isOK, const QString& objName, const QString& message);
};
