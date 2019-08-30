#pragma once
#include "Packet.h"
#include "Graphics.h"
#include "QTcpSocket"

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
	SeaBattle& _client;

	static QByteArray GetBytes(const Packet& packet);
	static QString GetErrorDescr(QAbstractSocket::SocketError err, const QString& alternate);

signals:

	void SignalReceive(Packet packet);

protected slots:

	void SlotClosed()
	{
		emit SignalReceive(Packet(Packet::STATE::DISCONNECTED));
	}
};
