#pragma once
#include "NetworkInterface.h"
#include "QTcpSocket"

class Client : protected NetworkInterface
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
	QString GetErrorString() const override;

protected:

	void SendToServer();

private:

	QTcpSocket _tcpSocket{this};
	QString _errorString;

private slots:

	void SlotReadyRead();
	void SlotError(QAbstractSocket::SocketError);
	void SlotConnected();
};
