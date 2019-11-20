#include "stdafx.h"
#include "Client.h"
#include "ClientThread.h"

using namespace std;

std::variant<Packet, NetworkInterface::STATUS> Client::IncomingProc(Packet packet)
{
	STATUS status = STATUS::NOTHING;
	if (!packet)
	{
		emit SignalReceive(move(packet), &status);
		return status;
	}

	switch (_currentState)
	{
	case STATE::PUSHMAP:
	{
		Packet out;
		out.WriteData(_graphics.GetData());
		_currentState = STATE::WAITMAP;
		return out;
	}
	case STATE::WAITMAP:
		if (!packet.ReadRivals(_graphics.GetData()))
		{
			emit SignalReceive(Packet("WAITMAP error."), &status);
			return status;
		}
		_currentState = STATE::HIT;
		emit SignalReceive(Packet(Packet::STATE::CONNECTED), &status);
		return status;
	case STATE::WAITHIT:
		quint8 coord;
		if (Packet::DOIT doit; !packet.ReadData(doit, coord) || doit != Packet::DOIT::HIT)
		{
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

void Client::Run()
{
	_tcpSocket = new ClientThread(this);

	Q_UNUSED(connect(_tcpSocket, SIGNAL(SigConnected(std::variant<Packet, NetworkInterface::STATUS>*)), SLOT(SlotConnected(std::variant<Packet, NetworkInterface::STATUS>*)), Qt::BlockingQueuedConnection));
	Q_UNUSED(connect(_tcpSocket, SIGNAL(readyRead()), _tcpSocket, SLOT(SlotReadServer()), Qt::DirectConnection));
	Q_UNUSED(connect(_tcpSocket, SIGNAL(SigReadServer(std::variant<Packet, NetworkInterface::STATUS>*)), SLOT(SlotReadServer(std::variant<Packet, NetworkInterface::STATUS>*)), Qt::BlockingQueuedConnection));
	Q_UNUSED(connect(_tcpSocket, SIGNAL(SigError(std::optional<QAbstractSocket::SocketError>)), SLOT(SlotError(std::optional<QAbstractSocket::SocketError>)), Qt::BlockingQueuedConnection));
	Q_UNUSED(connect(this, SIGNAL(SigSend(Packet)), _tcpSocket, SLOT(SlotSend(Packet)), Qt::BlockingQueuedConnection));
	Q_UNUSED(connect(this, SIGNAL(finished()), _tcpSocket, SLOT(deleteLater())));
	Q_UNUSED(connect(this, SIGNAL(SigClose()), _tcpSocket, SLOT(SlotClose())));

	_tcpSocket->connectToHost(_curIP, _curPort, QIODevice::ReadWrite, QAbstractSocket::NetworkLayerProtocol::IPv4Protocol);
}
