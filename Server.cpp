#include "stdafx.h"
#include "Server.h"

using namespace std;

Server::Server(Graphics& g, SeaBattle& c, QObject* parent) : NetworkInterface(g, c, parent)
{
	connect(&_server, SIGNAL(newConnection()), SLOT(SlotNewConnection()));
	connect(&_server, SIGNAL(acceptError(QAbstractSocket::SocketError)), SLOT(SlotError(QAbstractSocket::SocketError)));
	_currentState = STATE::WAITMAP;
}

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
			_socket.reset();
			emit SignalReceive(Packet("WAITMAP error."));
			break;
		}
		out.WriteData(_graphics.GetData());
		SendToClient(out);
		_currentState = STATE::WAITHIT;
		emit SignalReceive(Packet(Packet::STATE::CONNECTED));
		break;
	case STATE::WAITHIT:
		quint8 coord;
		if (Packet::DOIT doit; !packet.ReadData(doit, coord) || doit != Packet::DOIT::HIT)
		{
			_currentState = STATE::WAITMAP;
			_socket.reset();
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

void Server::SendToClient(const Packet& packet) const
{
	if (_socket)
		_socket->write(GetBytes(packet));
}

void Server::SlotNewConnection()
{
	_server.close();
	QTcpSocket* const pClientSocket = _server.nextPendingConnection();
	if (_socket)
	{
		pClientSocket->close();
		return;
	}
	connect(pClientSocket, SIGNAL(disconnected()), SLOT(SlotClosed()));
	connect(pClientSocket, SIGNAL(disconnected()), pClientSocket, SLOT(deleteLater()));
	connect(pClientSocket, SIGNAL(readyRead()), SLOT(SlotReadClient()));
	connect(pClientSocket, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(SlotError(QAbstractSocket::SocketError)));
	_socket.reset(pClientSocket);
	_currentState = STATE::WAITMAP;
}

void Server::SlotReadClient()
{
	QTcpSocket* const pClientSocket = qobject_cast<QTcpSocket*>(sender());
	if (!pClientSocket)
		return;
	QDataStream in(pClientSocket);
	in.setVersion(QDataStream::Qt_5_10);
	quint16 blockSize = 0;
	if (pClientSocket->bytesAvailable() < 2)
		return;
	in >> blockSize;
	if (pClientSocket->bytesAvailable() < blockSize)
		return;
	IncomingProc(Packet(in, blockSize));
}

void Server::SlotError(const QAbstractSocket::SocketError err)
{
	const QString eStr = _socket ? _socket->errorString() : "Unknown";
	IncomingProc(Packet(GetErrorDescr(err, eStr)));
}

void Server::SendHit(const quint8 coord)
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
	SendToClient(packet);
}

void Server::Listen(const quint16 port)
{
	if (_server.isListening())
		_server.close();
	if (!_server.listen(QHostAddress::Any, port))
		emit SignalReceive(Packet(_server.errorString()));
}
