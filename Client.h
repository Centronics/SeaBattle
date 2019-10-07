#pragma once
#include "NetworkInterface.h"
#include "QTcpSocket"
#include <variant>

class Client : public NetworkInterface
{
	Q_OBJECT

public:

	explicit Client(Graphics& g, QObject* parent, NetworkInterface** r) : NetworkInterface(g, parent, r) { }
	Client() = delete;
	virtual ~Client() = default;
	Client(const Client&) = delete;
	Client(Client&&) = delete;
	Client& operator=(const Client&) = delete;
	Client& operator=(Client&&) = delete;

	void Connect(const QString& ip, const quint16 port)
	{
		if (this == nullptr)
			return;
		quit();
		wait();
		_curIP = ip;
		_curPort = port;
		start(NormalPriority);
	}

protected:

	void Send(const Packet& packet) override
	{
		emit SigSend(packet);// Õ≈ —Œ≈ƒ»Õﬂ≈“—ﬂ (ÌÂ ÔÂÂ‰‡∏ÚÒˇ ÒË„Ì‡Î)
	}

private:

	QTcpSocket* _tcpSocket = nullptr;
	QString _curIP;
	quint16 _curPort = 0;
	void Run() override;
	std::variant<Packet, STATUS> IncomingProc(Packet packet);

private slots:

	void SlotReadyRead()
	{
		IncomingProc(Packet(*_tcpSocket));// Õ¿  À»≈Õ“≈ —“¿“”— Õ≈ Œ¡ÕŒ¬Àﬂ≈“—ﬂ
	}

	void SlotError(const std::optional<QAbstractSocket::SocketError> err)
	{
		if (!err || err == QAbstractSocket::RemoteHostClosedError)
			IncomingProc(Packet(Packet::STATE::DISCONNECTED));
		else
			IncomingProc(Packet(GetErrorDescr(*err)));
	}

	void SlotConnected(std::variant<Packet, NetworkInterface::STATUS>* sendMe)
	{
		_currentState = STATE::PUSHMAP;
		if (sendMe)
			*sendMe = IncomingProc(Packet());
	}

signals:

	void SigSend(Packet);
};
