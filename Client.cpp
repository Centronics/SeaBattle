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

inline void Client::SendHit(const quint8 coord)
{
	if (_currentState != STATE::HIT)
		return;
	if (!_graphics.MyHit(coord))
	{
		_currentState = STATE::WAITHIT;
		Graphics::IsRivalMove = true;
	}
	Packet packet;
	packet.WriteData(Packet::DOIT::HIT, coord);
	SendToServer(packet);
}

void Client::Close()
{
	_currentState = STATE::PUSHMAP;
	_tcpSocket.close();
}

void Client::Connect(const QString& ip, const quint16 port)
{
	_tcpSocket.close();
	_tcpSocket.connectToHost(ip, port, QIODevice::ReadWrite, QAbstractSocket::NetworkLayerProtocol::IPv4Protocol);
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

void Client::SlotReadyRead()
{
	QDataStream in(&_tcpSocket);
	in.setVersion(QDataStream::Qt_5_10);
	quint16 blockSize = 0;
	if (_tcpSocket.bytesAvailable() < 2)
		return;
	in >> blockSize;
	if (_tcpSocket.bytesAvailable() < blockSize)
		return;
	IncomingProc(Packet(in, blockSize));
}

void Client::SendToServer(const Packet& packet)
{
	_tcpSocket.write(GetBytes(packet));
}

void Client::SlotConnected()
{
	_currentState = STATE::PUSHMAP;
	IncomingProc(Packet());
}

void Client::SlotError(const QAbstractSocket::SocketError err)
{
	IncomingProc(Packet(GetErrorDescr(err, _tcpSocket.errorString())));
}
