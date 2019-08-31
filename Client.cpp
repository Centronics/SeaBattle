#include "stdafx.h"
#include "Client.h"

using namespace std;

Client::Client(Graphics& g, SeaBattle& c, QObject* parent) : NetworkInterface(g, c, parent)
{
	connect(&_tcpSocket, SIGNAL(connected()), SLOT(SlotConnected()));
	connect(&_tcpSocket, SIGNAL(readyRead()), SLOT(SlotReadyRead()));
	connect(&_tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(SlotError(QAbstractSocket::SocketError)));
	connect(&_tcpSocket, SIGNAL(disconnected()), SLOT(SlotClosed()));
	_currentState = STATE::PUSHMAP;
}

void Client::IncomingProc(Packet packet)
{
	if (!packet)
	{
		emit SignalReceive(move(packet));
		return;
	}
	switch (Packet out; _currentState)
	{
	case STATE::PUSHMAP:
		out.WriteData(_graphics.GetData());
		SendToServer(out);
		_currentState = STATE::WAITMAP;
		break;
	case STATE::WAITMAP:
		if (!packet.ReadRivals(_graphics.GetData()))
		{
			Close();
			emit SignalReceive(Packet("WAITMAP error."));
			break;
		}
		_currentState = STATE::WAITHIT;
		emit SignalReceive(Packet(Packet::STATE::CONNECTED));
		break;
	case STATE::WAITHIT:
		quint8 coord;
		if (Packet::DOIT doit; !packet.ReadData(doit, coord) || doit != Packet::DOIT::HIT)
		{
			Close();
			emit SignalReceive(Packet("HIT error."));
			break;
		}
		if (!_graphics.RivalHit(coord))
		{
			_currentState = STATE::HIT;
			Graphics::IsRivalMove = false;
		}
		emit SignalReceive(move(packet));
		break;
	case STATE::HIT:
		return;
	default:
		throw exception(__func__);
	}
}
