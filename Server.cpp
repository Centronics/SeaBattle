#include "stdafx.h"
#include "Server.h"
#include "ServerThread.h"

using namespace std;

std::variant<Packet, NetworkInterface::STATUS> Server::IncomingProc(Packet packet)
{
	STATUS status = STATUS::NOTHING;
	if (!packet)
	{
		emit SignalReceive(move(packet), &status);
		return status;
	}
	switch (_currentState)
	{
	case STATE::WAITMAP:
	{
		if (!packet.ReadRivals(_graphics.GetData()))
		{
			_currentState = STATE::WAITMAP;
			emit SignalReceive(Packet("WAITMAP error."), &status);
			return status;
		}
		Packet out;
		out.WriteData(_graphics.GetData());
		_currentState = STATE::WAITHIT;
		emit SignalReceive(Packet(Packet::STATE::CONNECTED), nullptr);
		return out;
	}
	case STATE::WAITHIT:
		quint8 coord;
		if (Packet::DOIT doit; !packet.ReadData(doit, coord) || doit != Packet::DOIT::HIT)
		{
			_currentState = STATE::WAITMAP;
			emit SignalReceive(Packet("HIT error."), &status);
			return status;
		}
		if (!_graphics.RivalHit(coord))
		{
			_currentState = STATE::HIT;
			Graphics::IsRivalMove = false;
		}
		emit Update();
		emit SignalReceive(move(packet), &status);
		return status;
	case STATE::HIT:
		return status;
	default:
		throw exception(__func__);
	}
}

void Server::Run()
{
	_server = new ServerThread(this);

	Q_UNUSED(connect(_server, SIGNAL(SigNewConnection()), SLOT(SlotNewConnection()), Qt::BlockingQueuedConnection));
	Q_UNUSED(connect(_server, SIGNAL(SigRead(QTcpSocket*, std::variant<Packet, NetworkInterface::STATUS>*)), SLOT(SlotReadClient(QTcpSocket*, std::variant<Packet, NetworkInterface::STATUS>*)), Qt::BlockingQueuedConnection));
	Q_UNUSED(connect(_server, SIGNAL(SigError(std::optional<QAbstractSocket::SocketError>)), SLOT(SlotError(std::optional<QAbstractSocket::SocketError>)), Qt::BlockingQueuedConnection));
	Q_UNUSED(connect(_server, SIGNAL(acceptError(QAbstractSocket::SocketError)), SLOT(SlotAcceptError(QAbstractSocket::SocketError)), Qt::BlockingQueuedConnection));
	Q_UNUSED(connect(this, SIGNAL(SigSend(Packet)), _server, SLOT(SlotSend(Packet)), Qt::BlockingQueuedConnection));
	Q_UNUSED(connect(this, SIGNAL(finished()), _server, SLOT(deleteLater())));
	Q_UNUSED(connect(this, SIGNAL(SigClose()), _server, SLOT(SlotClose())));

	if (!_server->listen(QHostAddress::Any, _port))
		emit SignalReceive(Packet(_server->errorString()), nullptr);
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