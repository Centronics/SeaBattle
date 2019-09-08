#pragma once
#include "Packet.h"
#include "Graphics.h"
#include "QTcpSocket"

class NetworkInterface : public QObject
{
	Q_OBJECT

public:

	explicit NetworkInterface(Graphics& g, QObject* parent) : QObject(parent), _graphics(g) { }
	NetworkInterface() = delete;
	virtual ~NetworkInterface() = default;
	NetworkInterface(const NetworkInterface&) = delete;
	NetworkInterface(NetworkInterface&&) = delete;
	NetworkInterface& operator=(const NetworkInterface&) = delete;
	NetworkInterface& operator=(NetworkInterface&&) = delete;

	[[nodiscard]] virtual std::optional<QString> SendHit()
	{
		const Packet p = CreateHitPacket();
		if (!p)
		{
			QString s;
			Q_UNUSED(p.GetState(&s));
			return s;
		}
		Send(p);
		return std::nullopt;
	}

protected:

	enum class STATE : quint8
	{
		PUSHMAP,
		WAITMAP,
		WAITHIT,
		HIT
	};

	STATE _currentState = STATE::PUSHMAP;
	Graphics& _graphics;

	[[nodiscard]] static QString GetErrorDescr(QAbstractSocket::SocketError err);
	[[nodiscard]] Packet CreateHitPacket();
	virtual void Send(const Packet& packet) = 0;

signals:

	void SignalReceive(Packet);
	void Update();

protected slots:

	void SlotClosed()
	{
		emit SignalReceive(Packet(Packet::STATE::DISCONNECTED));
	}
};
