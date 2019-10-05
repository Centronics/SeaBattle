#include "stdafx.h"
#include "Client.h"
#include "ClientThread.h"

using namespace std;

optional<Packet> Client::IncomingProc(Packet packet) // ÏÐÅÄËÀÃÀÞ ÂÎÇÂÐÀÙÀÒÜ NEEDCLEAN
{
	if (!packet)
	{
		emit SignalReceive(move(packet));
		return nullopt;
	}

	const auto close = [this]
	{
		_currentState = STATE::PUSHMAP;//ÍÅÏÐÀÂÈËÜÍÎ ÏÐÎÈÇÂÎÄÈÒÑß Î×ÈÑÒÊÀ
		_tcpSocket->close();
	};

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
			close();
			emit SignalReceive(Packet("WAITMAP error."));
			return nullopt;
		}
		_currentState = STATE::HIT;
		emit SignalReceive(Packet(Packet::STATE::CONNECTED));
		return nullopt;
	case STATE::WAITHIT:
		quint8 coord;
		if (Packet::DOIT doit; !packet.ReadData(doit, coord) || doit != Packet::DOIT::HIT)
		{
			close();
			emit SignalReceive(Packet("HIT error."));
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

void Client::Run()
{
	_tcpSocket = new ClientThread(this);
	
	connect(_tcpSocket, SIGNAL(SigConnected(std::optional<Packet>*)), SLOT(SlotConnected(std::optional<Packet>*)), Qt::BlockingQueuedConnection);
	connect(_tcpSocket, SIGNAL(readyRead()), SLOT(SlotReadyRead()), Qt::BlockingQueuedConnection);
	connect(_tcpSocket, SIGNAL(SigError(std::optional<QAbstractSocket::SocketError>)), SLOT(SlotError(std::optional<QAbstractSocket::SocketError>)), Qt::BlockingQueuedConnection);
	connect(this, SIGNAL(finished()), _tcpSocket, SLOT(deleteLater()));

	_tcpSocket->connectToHost(_curIP, _curPort, QIODevice::ReadWrite, QAbstractSocket::NetworkLayerProtocol::IPv4Protocol);
}
