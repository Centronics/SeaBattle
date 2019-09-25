#include "stdafx.h"
#include "Server.h"
#include <functional>

using namespace std;

void Server::IncomingProc(Packet packet)
{
	if (!packet)
	{
		emit SignalReceive(move(packet));
		return;
	}
	switch (Packet out; _currentState)
	{
	case STATE::WAITMAP:
		if (!packet.ReadRivals(_graphics.GetData()))
		{
			_currentState = STATE::WAITMAP;
			//_socket = nullptr;
			emit SigClose();
			emit SignalReceive(Packet("WAITMAP error."));
			return;
		}
		out.WriteData(_graphics.GetData());
		Send(out);
		_currentState = STATE::WAITHIT;
		emit SignalReceive(Packet(Packet::STATE::CONNECTED));
		return;
	case STATE::WAITHIT:
		quint8 coord;
		if (Packet::DOIT doit; !packet.ReadData(doit, coord) || doit != Packet::DOIT::HIT)
		{
			_currentState = STATE::WAITMAP;
			//_socket = nullptr;
			emit SigClose();
			emit SignalReceive(Packet("HIT error."));
			return;
		}
		if (!_graphics.RivalHit(coord))
		{
			_currentState = STATE::HIT;
			Graphics::IsRivalMove = false;
		}
		emit Update();
		return;
	case STATE::HIT:
		return;
	default:
		throw exception(__func__);
	}
}

void Server::Run()
{
	_server = new DoOnThread;
	
	
	auto t2 = connect(_server, SIGNAL(acceptError(QAbstractSocket::SocketError)),_server, SLOT(SlotError1(QAbstractSocket::SocketError)));

	auto t = connect(_server, SIGNAL(newConnection()), _server, SLOT(SlotNewConnection()));//–јЅќ“ј≈“
	auto t1 = connect(_server, SIGNAL(SigNewConnection()), SLOT(SlotNewConnection()));
	
	auto t3 =  connect(_server, SIGNAL(SigError(QAbstractSocket::SocketError)), SLOT(SlotError2(QAbstractSocket::SocketError)), Qt::BlockingQueuedConnection);
	auto t30 = connect(_server, SIGNAL(SigError1(QAbstractSocket::SocketError)), SLOT(SlotError2(QAbstractSocket::SocketError)), Qt::BlockingQueuedConnection);
	
	//auto t4 = connect(this, SIGNAL(SigSend(Packet)), _server, SLOT(SlotSend(Packet)));
	//auto t5 = connect(this, SIGNAL(SigClose()), SLOT(deleteLater()));
	//connect(_server, SIGNAL(SigClose()), _server, SLOT(SlotClose()));

	//auto t6 = connect(_server, SIGNAL(destroyed(QObject*)), _server, SLOT(Des(QObject*)));//¬озможно, что не вызываетс€, т.к. созданный в этом классе поток уже завершЄн (Ќ≈ ѕќ Ё“ќћ”)
	//auto t7 = connect(_server, SIGNAL(destroyed(QObject*)), SLOT(Destr(QObject*))); //ѕќ„≈ћ” —»√ЌјЋ »—ѕќЋЌя≈“—я ¬ √Ћј¬Ќќћ ѕќ“ќ ≈? »з-за того, что ему негде больше это делать.

	auto t8 = connect(this, SIGNAL(finished()), _server, SLOT(deleteLater()));

	//connect(this, SIGNAL(finished()), SLOT(testConn()));//¬џѕќЋЌя≈“—я ¬ √Ћј¬Ќќћ ѕќ“ќ ≈
	//auto t9 = connect(this, SIGNAL(finished()), _server, SLOT(testConn()));//¬џѕќЋЌя≈“—я ¬ ѕќ“ќ ≈ —≈–¬≈–ј

	//auto t10 = connect(this, SIGNAL(Do()), _server, SLOT(Do()));
	

	//connect(this, SIGNAL(SendToThread(function<void()>)), _server, SLOT(DoThis(function<void()>)));
	//connect(this, SIGNAL(Do()), _server, SLOT(Do()));
//	emit Do();
//
	if (!_server->listen(QHostAddress::Any, _port))
		emit SignalReceive(Packet(_server->errorString()));
}

void Server::SlotNewConnection()
{
	_currentState = STATE::WAITMAP;
}

void Server::SlotError2(QAbstractSocket::SocketError err)
{
	if (err == QAbstractSocket::RemoteHostClosedError)
		IncomingProc(Packet(Packet::STATE::DISCONNECTED));
	else
		IncomingProc(Packet(GetErrorDescr(err)));
}

void Server::Listen(const quint16 port)
{
	/*const auto f = [this, port]
	{
		if (_server->isListening())
			_server->close();
		if (!_server->listen(QHostAddress::Any, port))
			emit SignalReceive(Packet(_server->errorString()));
	};*/
	quit();
	wait();
	_port = port;
	start(NormalPriority);

	//msleep(3000);

	//_server->moveToThread(this);
	
	

	
	
	//emit SendToThread(f);
}
