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
	void Close() override;
	void Connect(const QString& ip, quint16 port);

private:

	QTcpSocket _tcpSocket{ this };

	void SendToServer(const Packet& packet);
	void IncomingProc(Packet packet);

private slots:

	void SlotReadyRead();
	void SlotError(QAbstractSocket::SocketError);
	void SlotConnected();
};
