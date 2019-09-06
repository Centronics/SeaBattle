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

	virtual void SendHit() = 0;

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

signals:

	void SignalReceive(Packet);
	void Update();

protected slots:

	void SlotClosed()
	{
		emit SignalReceive(Packet(Packet::STATE::DISCONNECTED));
	}
};
