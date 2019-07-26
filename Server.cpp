#include "stdafx.h"
#include "Server.h"

using namespace std;

void Server::Send()
{
	switch (_currentState)
	{
	case DOIT::STARTGAME:
		_packet.WriteData(DOIT::STARTGAME);
		//Œ“œ–¿¬ ¿

		break;
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

void Server::Receive()
{
	switch (_currentState)
	{
	case DOIT::STARTGAME:
		_currentState = DOIT::CONNECTIONERROR;
		_currentState = DOIT::PUSHMAP;
		_currentState = DOIT::STOPGAME;
		break;
	case DOIT::PUSHMAP:
		_currentState = DOIT::CONNECTIONERROR;
		_currentState = DOIT::PUSHMAP;
		_currentState = DOIT::STOPGAME;
		break;
	case DOIT::STOPGAME:
		break;
	case DOIT::HIT:
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

void Server::SendToClient(QTcpSocket* socket) const
{
	QByteArray arrBlock;
	QDataStream out(&arrBlock, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_5_10);
	out << quint16(0);
	if (!_packet.WriteToQDataStream(out))
		return;
	out.device()->seek(0);
	out << quint16(arrBlock.size() - 2);
	socket->write(arrBlock);
}

optional<Packet> Server::GetFromQueue()
{
	lock_guard locker(_lock);
	if (_requests.empty())
		return nullopt;
	const Packet element = _requests.front();
	_requests.pop();
	return element;
}

void Server::SlotNewConnection()
{
	QTcpSocket* pClientSocket = _server.nextPendingConnection();
	connect(pClientSocket, SIGNAL(disconnected()), pClientSocket, SLOT(deleteLater()));
	connect(pClientSocket, SIGNAL(readyRead()), this, SLOT(SlotReadClient()));
	SendToClient(pClientSocket);
}

void Server::SlotReadClient()
{
	auto* pClientSocket = dynamic_cast<QTcpSocket*>(sender());
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
	lock_guard locker(_lock);
	_requests.emplace(in, blockSize);
}

Server::Server(Graphics& g, SeaBattle& c, QObject* const parent) : NetworkInterface(g, c, parent)
{
	connect(&_server, SIGNAL(newConnection()), this, SLOT(SlotNewConnection()));
}

void Server::SendHit(const quint8 coord)
{
	if (_currentState != DOIT::MYMOVE)
		return;
	_packet.WriteData(DOIT::HIT, coord);
}

bool Server::Listen(const int port)
{
	if (_server.isListening())
		_server.close();
	return _isReady = _server.listen(QHostAddress::Any, port);
}
