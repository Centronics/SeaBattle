#include "stdafx.h"
#include "Server.h"

using namespace std;

void Server::SendAnswerToClient(const Packet& packet)
{
	switch (Packet out; _currentState)
	{
	case DOIT::STARTGAME:
		if (DOIT doit; !packet.ReadData(doit) && doit != DOIT::STARTGAME)
		{
			SocketClose();
			return;
		}
		out.WriteData(DOIT::STARTGAME);
		SendToClient(out);
		_currentState = DOIT::WAITMAP;
		break;
	case DOIT::WAITMAP:
		if (DOIT doit; !packet.ReadData(doit) || doit != DOIT::PUSHMAP)
		{
			_currentState = DOIT::STARTGAME;
			SocketClose();
			emit SignalReceive(Packet("WAITMAP error."));
			break;
		}
		if (!_graphics.ReadRivals(packet))
		{
			_currentState = DOIT::STARTGAME;
			SocketClose();
			emit SignalReceive(Packet("Rivals not readed."));
			break;
		}
		out.WriteData(_graphics.GetData());
		SendToClient(out);
		_currentState = DOIT::WAITHIT;
		emit SignalReceive(Packet());
		break;
	case DOIT::WAITHIT:
		if (DOIT doit; !packet.ReadData(doit) || doit != DOIT::HIT)
		{
			_currentState = DOIT::STARTGAME;
			SocketClose();
			emit SignalReceive(Packet("HIT error."));
			break;
		}
		_currentState = DOIT::HIT;
		emit SignalReceive(packet);
		break;
	case DOIT::HIT:
		break;
	default:
		throw exception(__func__);
	}
}

void Server::SocketClose()
{
	if (!_socket)
		return;
	_socket->close();
	_socket = nullptr;
}

void Server::SendToClient(const Packet& packet) const
{
	if (_socket)
		_socket->write(GetBytes(packet));//Проверить ошибку??
}

void Server::SlotNewConnection()
{
	QTcpSocket* const pClientSocket = _server.nextPendingConnection();
	connect(pClientSocket, SIGNAL(disconnected()), pClientSocket, SLOT(deleteLater()));
	connect(pClientSocket, SIGNAL(readyRead()), SLOT(SlotReadClient()));
	if (_socket)
		pClientSocket->close();
	else
	{
		_socket = pClientSocket;
		_currentState = DOIT::STARTGAME;
	}
}

void Server::SlotReadClient()
{
	QTcpSocket* const pClientSocket = dynamic_cast<QTcpSocket*>(sender());
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
	SendAnswerToClient(Packet(in, blockSize));
}

void Server::SendHit(const quint8 coord)
{
	if (_currentState != DOIT::HIT)
		return;
	Packet packet;
	packet.WriteData(DOIT::HIT, coord);
	SendToClient(packet);
}

void Server::Close()
{
	_server.close();
	SocketClose();
	_currentState = DOIT::STARTGAME;
}

void Server::Listen(const quint16 port)
{
	if (_server.isListening())
		_server.close();
	if (!_server.listen(QHostAddress::Any, port))
		emit SignalReceive(Packet(_server.errorString()));
}
