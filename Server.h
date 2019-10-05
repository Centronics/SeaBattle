#pragma once
#include "NetworkInterface.h"
#include "QTcpServer"

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

	QTcpServer* _server = nullptr;

	std::optional<Packet> IncomingProc(Packet packet);
	void Run() override;
	quint16 _port = 0;

protected:

	void Send(const Packet& packet) override
	{
		emit SigSend(packet);
	}

private slots:

	void SlotNewConnection();

	void SlotReadClient(QTcpSocket* socket, std::optional<Packet>* sendMe)
	{
		std::optional<Packet> packet = IncomingProc(Packet(*socket));
		if (sendMe)
			*sendMe = std::move(packet);
	}

	void SlotError(const std::optional<QAbstractSocket::SocketError> err)
	{
		if (!err || err == QAbstractSocket::RemoteHostClosedError)
			IncomingProc(Packet(Packet::STATE::DISCONNECTED));
		else
			IncomingProc(Packet(GetErrorDescr(*err)));
		Close();
	}

	void SlotAcceptError(const QAbstractSocket::SocketError err)
	{
		IncomingProc(Packet(GetErrorDescr(err)));
		Close();
	}

signals:

	void SigSend(Packet);
};