#include "stdafx.h"
#include "Client.h"
#include "ClientThread.h"

using namespace std;

void Client::IncomingProc(Packet packet)
{
	if (!packet)
	{
		emit SignalReceive(move(packet));
		return;
	}

	const auto close = [this]
	{
		_currentState = STATE::PUSHMAP;
		_tcpSocket->close();
	};

	switch (Packet out; _currentState)
	{
	case STATE::PUSHMAP:
		out.WriteData(_graphics.GetData());
		Send(out);
		_currentState = STATE::WAITMAP;
		break;
	case STATE::WAITMAP:
		if (!packet.ReadRivals(_graphics.GetData()))
		{
			close();
			emit SignalReceive(Packet("WAITMAP error."));
			break;
		}
		_currentState = STATE::HIT;
		emit SignalReceive(Packet(Packet::STATE::CONNECTED));
		break;
	case STATE::WAITHIT:
		quint8 coord;
		if (Packet::DOIT doit; !packet.ReadData(doit, coord) || doit != Packet::DOIT::HIT)
		{
			close();
			emit SignalReceive(Packet("HIT error."));
			break;
		}
		if (!_graphics.RivalHit(coord))
		{
			_currentState = STATE::HIT;
			Graphics::IsRivalMove = false;
		}
		emit Update();
		break;
	case STATE::HIT:
		return;
	default:
		throw exception(__func__);
	}
}

void Client::Run()
{
	_tcpSocket = new ClientThread;

	connect(_tcpSocket, SIGNAL(connected()), SLOT(SlotConnected()), Qt::BlockingQueuedConnection);
	connect(_tcpSocket, SIGNAL(readyRead()), SLOT(SlotReadyRead()), Qt::BlockingQueuedConnection);
	connect(this, SIGNAL(SigSend(Packet)), _tcpSocket, SLOT(SlotSend(Packet)));
	connect(this, SIGNAL(finished()), _tcpSocket, SLOT(deleteLater()));

	_tcpSocket->connectToHost(_curIP, _curPort, QIODevice::ReadWrite, QAbstractSocket::NetworkLayerProtocol::IPv4Protocol);
}
