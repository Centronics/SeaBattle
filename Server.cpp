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
			_socket = nullptr;
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
			_socket = nullptr;
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
	connect(_server, SIGNAL(newConnection()), SLOT(SlotNewConnection()));
	connect(_server, SIGNAL(acceptError(QAbstractSocket::SocketError)), SLOT(SlotError(QAbstractSocket::SocketError)));

	connect(_server, SIGNAL(destroyed(QObject*)), _server, SLOT(Des(QObject*)));//Âîçìîæíî, ÷òî íå âûçûâàåòñÿ, ò.ê. ñîçäàííûé â ýòîì êëàññå ïîòîê óæå çàâåðø¸í (ÍÅ ÏÎ ÝÒÎÌÓ)
	connect(_server, SIGNAL(destroyed(QObject*)), SLOT(Destr(QObject*))); //ÏÎ×ÅÌÓ ÑÈÃÍÀË ÈÑÏÎËÍßÅÒÑß Â ÃËÀÂÍÎÌ ÏÎÒÎÊÅ?
	
	connect(this, SIGNAL(finished()), _server, SLOT(deleteLater()));

	//connect(this, SIGNAL(finished()), SLOT(testConn()));//ÂÛÏÎËÍßÅÒÑß Â ÃËÀÂÍÎÌ ÏÎÒÎÊÅ
	connect(this, SIGNAL(finished()), _server, SLOT(Do()));//ÂÛÏÎËÍßÅÒÑß Â ÏÎÒÎÊÅ ÑÅÐÂÅÐÀ
	
	connect(this, SIGNAL(Do()), _server, SLOT(Do()));
	if (!_server->listen(QHostAddress::Any, _port))
		emit SignalReceive(Packet(_server->errorString()));
	
	//connect(this, SIGNAL(SendToThread(function<void()>)), _server, SLOT(DoThis(function<void()>)));
	//connect(this, SIGNAL(Do()), _server, SLOT(Do()));
	emit Do();
}

void Server::SlotNewConnection()
{
	QTcpSocket* const pClientSocket = _server->nextPendingConnection();
	if (_socket)
	{
		const Packet p(Packet::STATE::BUSY);
		p.Send(*pClientSocket);
		pClientSocket->close();
		return;
	}
	connect(pClientSocket, SIGNAL(disconnected()), pClientSocket, SLOT(deleteLater()));

	connect(pClientSocket, SIGNAL(disconnected()), SLOT(testConn1()));
	//connect(this, SIGNAL(finished()), pClientSocket, SLOT(deleteLater())); // ÍÓÆÍÎ ËÈ ÝÒÎ?
	
	//connect(pClientSocket, SIGNAL(disconnected()), SLOT(IntClose())); // ÒÎ×ÍÎ ËÈ ÐÀÁÎÒÀÅÒ?
	connect(pClientSocket, SIGNAL(readyRead()), SLOT(SlotReadClient()));
	connect(pClientSocket, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(SlotError(QAbstractSocket::SocketError)));
	_socket = pClientSocket;
	_currentState = STATE::WAITMAP;
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
	//emit SendToThread(f);
}
