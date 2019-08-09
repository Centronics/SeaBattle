#pragma once
#include "Packet.h"
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
	virtual void Close() = 0;

protected:

	DOIT _currentState = DOIT::STARTGAME;
	Graphics& _graphics;
	SeaBattle& _client;

	static QByteArray GetBytes(const Packet& packet)
	{
		QByteArray arrBlock;
		QDataStream out(&arrBlock, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_5_10);
		out << quint16(0);
		if (!packet.SerializeToQDataStream(out))
			throw std::exception("Не могу сериализовать пакет.");
		out.device()->seek(0);
		out << quint16(arrBlock.size() - 2);
		return arrBlock;
	}

signals:

	void SignalReceive(const Packet& packet);
};
