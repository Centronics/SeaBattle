#pragma once
#include "NetworkInterface.h"

class Server : public NetworkInterface
{
	Q_OBJECT

public:

	explicit Server(Graphics& g, QObject* parent, NetworkInterface** r) : NetworkInterface(g, parent, r) { }
	Server() = delete;
	virtual ~Server() = default;
	Server(const Server&) = delete;
	Server(Server&&) = delete;
	Server& operator=(const Server&) = delete;
	Server& operator=(Server&&) = delete;

	void Listen(quint16 port);

private:

	ServerThread* _server = nullptr;

	void IncomingProc(Packet packet);
	void Run() override;
	quint16 _port = 0;

protected:

	void Send(const Packet& packet) override
	{
		emit SigSend(packet);
	}

private slots:

	void SlotNewConnection();

	void SlotReadClient()
	{
		IncomingProc(Packet(*qobject_cast<QTcpSocket*>(sender())));
	}

	void SlotError(const QAbstractSocket::SocketError err)
	{
		if (err == QAbstractSocket::RemoteHostClosedError)
			IncomingProc(Packet(Packet::STATE::DISCONNECTED));
		else
			IncomingProc(Packet(GetErrorDescr(err)));
	}

signals:

	void SigSend(Packet);
};