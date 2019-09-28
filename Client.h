#pragma once
#include "NetworkInterface.h"
#include "QTcpSocket"

class Client : public NetworkInterface
{
	Q_OBJECT

public:

	explicit Client(Graphics& g, QObject* parent, NetworkInterface** r);
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

		/*const auto f = [this, ip, port]
		{

		};

		emit SendToThread(f);*/
	}

protected:

	void Send(const Packet& packet) override
	{
		packet.Send(*_tcpSocket);
	}

private:

	QTcpSocket* _tcpSocket = nullptr;
	void IncomingProc(Packet packet);
	void Run() override;
	QString _curIP;
	quint16 _curPort = 0;

private slots:

	void SlotDeleteMe()
	{
		//	IntClose();
	}

	void SlotReadyRead()
	{
		IncomingProc(Packet(*_tcpSocket));
	}

	void SlotError(const QAbstractSocket::SocketError err)
	{
		if (err == QAbstractSocket::RemoteHostClosedError)
			IncomingProc(Packet(Packet::STATE::DISCONNECTED));
		else
			IncomingProc(Packet(GetErrorDescr(err)));
	}

	void SlotConnected()
	{
		_currentState = STATE::PUSHMAP;
		IncomingProc(Packet());
	}
};
