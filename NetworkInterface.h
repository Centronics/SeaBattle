#pragma once
#include "Packet.h"
#include "Graphics.h"

class SeaBattle;

class NetworkInterface : public QObject
{
	Q_OBJECT

public:

	explicit NetworkInterface(Graphics& g, SeaBattle& c, QObject* parent, const std::vector<Ship>& mapData) : QObject(parent), _graphics(g), _client(c), _mapData(mapData) { }
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

	DOIT _currentState = DOIT::STARTGAME;
	Graphics& _graphics;
	SeaBattle& _client;
	const std::vector<Ship>& _mapData;

	[[nodiscard]] Packet RivalFlagInvert() const
	{
		std::vector<Ship> t = _mapData;
		std::for_each(t.begin(), t.end(), [](Ship& k)
		{
			switch (k.GetHolder())
			{
			case Ship::SHIPHOLDER::ME:
				k.SetHolder(Ship::SHIPHOLDER::RIVAL);
				return;
			case Ship::SHIPHOLDER::RIVAL:
				k.SetHolder(Ship::SHIPHOLDER::ME);
				return;
			default:
				throw std::exception("RivalFlagInvert");
			}
		});
		Packet packet;
		packet.WriteData(t);
		return packet;
	}

signals:

	void Connected(bool isOK, const QString& objName, const QString& message);
};
