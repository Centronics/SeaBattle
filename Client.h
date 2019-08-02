#pragma once
#include "NetworkInterface.h"
#include "QTcpSocket"

class SeaBattle;

class Client : public NetworkInterface
{
	Q_OBJECT

public:

	explicit Client(Graphics& g, SeaBattle& c, QObject* parent);
	Client() = delete;
	virtual ~Client() = default;
	Client(const Client&) = delete;
	Client(Client&&) = delete;
	Client& operator=(const Client&) = delete;
	Client& operator=(Client&&) = delete;

	void SendHit(quint8 coord) override;
	void Connect(const QString& ip, quint16 port);

protected:

	void SendToServer(const Packet& packet);

private:

	QTcpSocket _tcpSocket{ this };
	void Send(const Packet* packet = nullptr);

private slots:

	void SlotReadyRead();
	void SlotError(QAbstractSocket::SocketError);
	void SlotConnected();
};
