#include "stdafx.h"
#include "Client.h"

using namespace std;

Client::Client(Graphics& g, QObject* parent, NetworkInterface** r) : NetworkInterface(g, parent, r)
{
	connect(&_tcpSocket, SIGNAL(connected()), SLOT(SlotConnected()));
	connect(&_tcpSocket, SIGNAL(readyRead()), SLOT(SlotReadyRead()));
	connect(&_tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(SlotError(QAbstractSocket::SocketError)));
	//connect(&_tcpSocket, SIGNAL(disconnected()), SLOT(SlotClosed()));
}

void Client::Close()
{
	if (_closed)
		return;
	_tcpSocket.close();
	deleteLater();
	*_myRef = nullptr;
	_closed = true;
}

void Client::Connect(const QString& ip, const quint16 port)
{
	_tcpSocket.close();
	_tcpSocket.connectToHost(ip, port, QIODevice::ReadWrite, QAbstractSocket::NetworkLayerProtocol::IPv4Protocol);
	connect(&_tcpSocket, SIGNAL(disconnected()), SLOT(SlotDeleteMe()));
	connect(this, SIGNAL(NeedDelete()), &_tcpSocket, SLOT(deleteLater()));
}

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
		_tcpSocket.close();
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
