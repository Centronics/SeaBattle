#include "stdafx.h"
#include "Server.h"

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

void Server::SlotNewConnection()
{
	QTcpSocket* const pClientSocket = _server.nextPendingConnection();
	//_server.close();
	if (_socket)
	{
		const Packet p(Packet::STATE::BUSY);
		p.Send(*pClientSocket);
		pClientSocket->close();
		return;
	}
	//connect(pClientSocket, SIGNAL(disconnected()), SLOT(SlotClosed()));
	connect(pClientSocket, SIGNAL(disconnected()), pClientSocket, SLOT(deleteLater()));
	connect(pClientSocket, SIGNAL(readyRead()), SLOT(SlotReadClient()));
	connect(pClientSocket, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(SlotError(QAbstractSocket::SocketError)));
	_socket = pClientSocket;
	_currentState = STATE::WAITMAP;
}

void Server::Close()
{
	if (_closed)
		return;
	_server.close();
	deleteLater();
	*_myRef = nullptr;
	_closed = true;
}

void Server::Listen(const quint16 port)
{
	if (_server.isListening())
		_server.close();
	if (!_server.listen(QHostAddress::Any, port))
		emit SignalReceive(Packet(_server.errorString()));
}
