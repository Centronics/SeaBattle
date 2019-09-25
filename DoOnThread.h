#pragma once
#include "QTcpServer"
#include "Packet.h"
#include "qabstractsocket.h"

class DoOnThread : public QTcpServer
{
	Q_OBJECT

public:

	DoOnThread() = default;
	DoOnThread(const DoOnThread&) = delete;
	DoOnThread(DoOnThread&&) = delete;
	DoOnThread& operator=(const DoOnThread&) = delete;
	DoOnThread& operator=(DoOnThread&&) = delete;
	virtual ~DoOnThread() = default;

protected:

	QTcpSocket* _socket = nullptr;

signals:

	void SigNewConnection();
	void SigError(QAbstractSocket::SocketError);
	void SigError1(QAbstractSocket::SocketError);

public slots:

	/*void SlotClose()
{
	/*QTcpSocket* const s = _socket;
	if(s)
	{
	_socket = nullptr;
	s->deleteLater();
	}
	deleteLater();
}*/

	void SlotError1(QAbstractSocket::SocketError err)
	{
	emit SigError((QAbstractSocket::SocketError)0x202202);//err);
}

	// ReSharper disable once CppMemberFunctionMayBeConst
	void SlotSend(const Packet packet)  // NOLINT(performance-unnecessary-value-param)
	{
	QTcpSocket* const s = _socket;
		if (s)
			packet.Send(*s);
	}

	// ReSharper disable once CppMemberFunctionMayBeStatic
	/*void DoThis(const std::function<void()> f)
	{
		f();//Õ≈ ¬€œŒÀÕﬂ≈“—ﬂ
	}

	void Do()
	{//¬€œŒÀÕﬂ≈“—ﬂ
		//close();
		//QThread::currentThread()->quit();

	//deleteLater();
	}

	void Des(QObject*)
	{//Õ≈“  ŒÕ“¿ “¿!

	}*/

	void testConn()
	{
		
	}

	void SlotNewConnection()
	{
		QTcpSocket* const pClientSocket = nextPendingConnection();
		if (_socket)
		{
			const Packet p(Packet::STATE::BUSY);
			p.Send(*pClientSocket);
			pClientSocket->close();
			return;
		}
		connect(pClientSocket, SIGNAL(disconnected()), pClientSocket, SLOT(deleteLater()));

		connect(pClientSocket, SIGNAL(disconnected()), SLOT(testConn1()));
		//connect(this, SIGNAL(finished()), pClientSocket, SLOT(deleteLater())); // Õ”∆ÕŒ À» ›“Œ?

		//connect(pClientSocket, SIGNAL(disconnected()), SLOT(IntClose())); // “Œ◊ÕŒ À» –¿¡Œ“¿≈“?
		connect(pClientSocket, SIGNAL(readyRead()), SLOT(SlotReadClient()));
		connect(pClientSocket, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(SlotError1(QAbstractSocket::SocketError)));

		connect(this, SIGNAL(SigError1(QAbstractSocket::SocketError)), SLOT(SlotError1(QAbstractSocket::SocketError)));
	
		_socket = pClientSocket;
		emit SigNewConnection();

	emit SigError1((QAbstractSocket::SocketError)0x101101);
	}
};

