#include "stdafx.h"
#include "Server.h"

using namespace std;

void Server::Send(const Packet& packet)
{
	switch (_currentState)
	{
	case DOIT::STARTGAME:
	{
		DOIT doit;
		if (!packet.ReadData(doit))
			if (doit != DOIT::STARTGAME)
			{
				_socket->close();
				_socket = nullptr;
				return;
			}
		Packet out;
		out.WriteData(DOIT::STARTGAME);
		SendToClient(out);
		_currentState = DOIT::PUSHMAP;
		break;
	}
	case DOIT::PUSHMAP:

		break;
	case DOIT::STOPGAME:

		break;
	case DOIT::HIT:
		_currentState = DOIT::WAITRIVAL;
		break;
	case DOIT::CONNECTIONERROR:

		break;
	case DOIT::WAITRIVAL:

		break;
	case DOIT::MYMOVE:

		break;
	default:
		throw exception(__func__);
	}
}

void Server::SendToClient(const Packet& packet) const
{
	if (!_socket)
		return;
	QByteArray arrBlock;
	QDataStream out(&arrBlock, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_5_10);
	out << quint16(0);
	if (!packet.WriteToQDataStream(out))
		return;
	out.device()->seek(0);
	out << quint16(arrBlock.size() - 2);
	_socket->write(arrBlock);
}

void Server::SlotNewConnection()
{
	QTcpSocket* const pClientSocket = _server.nextPendingConnection();
	connect(pClientSocket, SIGNAL(disconnected()), pClientSocket, SLOT(deleteLater()));
	connect(pClientSocket, SIGNAL(readyRead()), SLOT(SlotReadClient()));
	if (_socket)
	{
		_socket->close();
		_socket = nullptr;
		return;
	}
	_socket = pClientSocket;
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
	Send(Packet(in, blockSize));
}

Server::Server(Graphics& g, SeaBattle& c, QObject* const parent, const vector<Ship>& mapData) : NetworkInterface(g, c, parent, mapData)
{
	connect(&_server, SIGNAL(newConnection()), SLOT(SlotNewConnection()));
}

void Server::SendHit(const quint8 coord)
{
	if (_currentState != DOIT::MYMOVE)
		return;
	Packet packet;
	packet.WriteData(DOIT::HIT, coord);
	SendToClient(packet);
}

void Server::Listen(const quint16 port)
{
	if (_server.isListening())
		_server.close();
	if (_server.listen(QHostAddress::Any, port))
		emit Connected(true, "Сервер.", QString());
	else
		emit Connected(false, "Сервер.", _server.errorString());
}
