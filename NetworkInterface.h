#pragma once
#include "Packet.h"
#include "Graphics.h"
#include "QTcpSocket"

class NetworkInterface : public QObject
{
	Q_OBJECT

public:

	explicit NetworkInterface(Graphics& g, QObject* parent, NetworkInterface** r) : QObject(parent), _graphics(g), _myRef(r) { }
	NetworkInterface() = delete;
	virtual ~NetworkInterface() = default;
	NetworkInterface(const NetworkInterface&) = delete;
	NetworkInterface(NetworkInterface&&) = delete;
	NetworkInterface& operator=(const NetworkInterface&) = delete;
	NetworkInterface& operator=(NetworkInterface&&) = delete;

	[[nodiscard]] std::optional<QString> SendHit();
	virtual void Close() = 0;

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
	bool _closed = true;
	NetworkInterface** const _myRef = nullptr;

	[[nodiscard]] static QString GetErrorDescr(QAbstractSocket::SocketError err);
	[[nodiscard]] std::optional<Packet> CreateHitPacket();
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
