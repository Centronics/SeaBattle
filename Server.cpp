#include "stdafx.h"
#include "Server.h"

using namespace std;

optional<Packet> Server::IncomingProc(Packet packet)
{
	if (!packet)
	{
		emit SignalReceive(move(packet));
		Close();
		return nullopt;
	}
	switch (_currentState)
	{
	case STATE::WAITMAP:
	{
		if (!packet.ReadRivals(_graphics.GetData()))
		{
			_currentState = STATE::WAITMAP;
			emit SignalReceive(Packet("WAITMAP error."));
			Close();
			return nullopt;
		}
		Packet out;
		out.WriteData(_graphics.GetData());
		//Send(out);
		_currentState = STATE::WAITHIT;
		emit SignalReceive(Packet(Packet::STATE::CONNECTED));
		return out;
	}
	case STATE::WAITHIT:
		quint8 coord;
		if (Packet::DOIT doit; !packet.ReadData(doit, coord) || doit != Packet::DOIT::HIT)
		{
			_currentState = STATE::WAITMAP;
			emit SignalReceive(Packet("HIT error."));
			Close();
			return nullopt;
		}
		if (!_graphics.RivalHit(coord))
		{
			_currentState = STATE::HIT;
			Graphics::IsRivalMove = false;
		}
		emit Update();
		return nullopt;
	case STATE::HIT:
		return nullopt;
	default:
		throw exception(__func__);
	}
}

void Server::Run()
{
	_server = new ServerThread(this);

	connect(_server, SIGNAL(SigNewConnection()), SLOT(SlotNewConnection()), Qt::BlockingQueuedConnection);
	connect(_server, SIGNAL(SigRead(QTcpSocket*, std::optional<Packet>*)), SLOT(SlotReadClient(QTcpSocket*, std::optional<Packet>*)), Qt::BlockingQueuedConnection);
	connect(this, SIGNAL(SigSend(Packet)), _server, SLOT(SlotSend(Packet)));
	connect(_server, SIGNAL(SigError(optional<QAbstractSocket::SocketError>)), SLOT(SlotError(optional<QAbstractSocket::SocketError>)), Qt::BlockingQueuedConnection);// ÍÅ ÑÎÅÄÈÍßÅÒÑß
	connect(_server, SIGNAL(acceptError(QAbstractSocket::SocketError)), SLOT(SlotAcceptError(QAbstractSocket::SocketError)), Qt::BlockingQueuedConnection);
	connect(this, SIGNAL(finished()), _server, SLOT(deleteLater()));

	if (!_server->listen(QHostAddress::Any, _port))
		emit SignalReceive(Packet(_server->errorString()));
}

void Server::SlotNewConnection()
{
	_currentState = STATE::WAITMAP;
}

void Server::Listen(const quint16 port)
{
	if (this == nullptr)
		return;
	quit();
	wait();
	_port = port;
	start(NormalPriority);
}
