#pragma once
#include "NetworkInterface.h"
#include "QTcpServer"
#include "QTcpSocket"

class Server : public NetworkInterface
{
	Q_OBJECT

	friend class DoOnThread;

public:

	explicit Server(Graphics& g, QObject* parent, NetworkInterface** r) : NetworkInterface(g, parent, r)
	{
		//connect(this, SIGNAL(SignalReceive(Packet)), parent, SLOT(SlotReceive(Packet)));
		//start(NormalPriority);
		//while(!isRunning())
			//sleep(1);
	}

	Server() = delete;
	virtual ~Server() = default;
	Server(const Server&) = delete;
	Server(Server&&) = delete;
	Server& operator=(const Server&) = delete;
	Server& operator=(Server&&) = delete;

	void Listen(quint16 port);

private:

	DoOnThread* _server = nullptr;//QTcpServer
	
	void IncomingProc(Packet packet);
	void Run() override;
	quint16 _port = 0;

	void IntClose() override//можно это сделать и в этом потоке
	{
		//if (_server)
			//_server->close();
		deleteLater();
	}

protected:

	void Send(const Packet& packet) override
	{
		//if (_socket)
			//packet.Send(*_socket);
			//
			emit SigSend(packet);
	}

	//signals:

	
	
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

	void testConn()
	{
		
	}

	void testConn1()
	{
		
	}

	void Destr(QObject*)
	{
		
	}
	
	/*void SlotDisconnect()
	{
		IntClose();
		//_socket->deleteLater();
	}*/

private:

	signals:

	void oo();//public
	void SigSend(Packet);
	void SigClose();
};

/*class DF : Server
{
	void f()
	{
		emit oo();
		//SlotReadClient();
	}
};*/