#include "stdafx.h"
#include "Server.h"
#include "ServerThread.h"

using namespace std;

std::variant<Packet, NetworkInterface::STATUS> Server::IncomingProc(Packet packet)
{
	if (!packet)
	{
		emit SignalReceive(move(packet));
		return STATUS::NOTHING;
	}
	switch (_currentState)
	{
	case STATE::WAITMAP:
	{
		if (!packet.ReadRivals(_graphics.GetData()))
		{
			_currentState = STATE::WAITMAP;
			emit SignalReceive(Packet("WAITMAP error."));
			//Close();//Õ≈œ–¿¬»À‹ÕŒ œ–Œ»«¬Œƒ»“—ﬂ Œ◊»—“ ¿
			return STATUS::NEEDCLEAN;
		}
		Packet out;
		out.WriteData(_graphics.GetData());
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
			//Close();//Õ≈œ–¿¬»À‹ÕŒ œ–Œ»«¬Œƒ»“—ﬂ Œ◊»—“ ¿
			return STATUS::NEEDCLEAN;
		}
		if (!_graphics.RivalHit(coord))
		{
			_currentState = STATE::HIT;
			Graphics::IsRivalMove = false;
		}
		emit Update();
		return STATUS::NOTHING;
	case STATE::HIT:
		return STATUS::NOTHING;
	default:
		throw exception(__func__);
	}
}

void Server::Run()
{
	_server = new ServerThread(this);

	connect(_server, SIGNAL(SigNewConnection()), SLOT(SlotNewConnection()), Qt::BlockingQueuedConnection);
	connect(_server, SIGNAL(SigRead(QTcpSocket*, std::variant<Packet, NetworkInterface::STATUS>*)), SLOT(SlotReadClient(QTcpSocket*, std::variant<Packet, NetworkInterface::STATUS>*)), Qt::BlockingQueuedConnection);
	connect(_server, SIGNAL(SigError(std::optional<QAbstractSocket::SocketError>)), SLOT(SlotError(std::optional<QAbstractSocket::SocketError>)), Qt::BlockingQueuedConnection);
	connect(_server, SIGNAL(acceptError(QAbstractSocket::SocketError)), SLOT(SlotAcceptError(QAbstractSocket::SocketError)), Qt::BlockingQueuedConnection);
	connect(this, SIGNAL(SigSend(Packet)), _server, SLOT(SlotSend(Packet)));
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
