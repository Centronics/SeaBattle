#include "stdafx.h"
#include "ClientServer.h"

using namespace std;

void ClientServer::Send()
{
	switch (_currentState)
	{
	case DOIT::STARTGAME:
		_senderPacket.WriteData(DOIT::STARTGAME);
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

void ClientServer::Send(const Packet& packet)
{

}

void ClientServer::Receive()
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

void ClientServer::SendToClient(QTcpSocket* socket) const
{
	QByteArray arrBlock;
	QDataStream out(&arrBlock, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_5_10);
	out << quint16(0);
	if (!_senderPacket.WriteToQDataStream(out))
		return;
	out.device()->seek(0);
	out << quint16(arrBlock.size() - 2);
	socket->write(arrBlock);
}

bool ClientServer::StartClient(const QString& ip, int port)
{
	return true;
}

bool ClientServer::StartServer(const int port)
{
	connect(&_server, SIGNAL(newConnection()), this, SLOT(SlotNewConnection()));
	return _server.listen(QHostAddress::Any, port);
}

optional<Packet> ClientServer::GetFromQueue()
{
	lock_guard locker(_lock);
	if (_requests.empty())
		return nullopt;
	const Packet element = _requests.front();
	_requests.pop();
	return element;
}

void ClientServer::SlotNewConnection()
{
	QTcpSocket* pClientSocket = _server.nextPendingConnection();
	connect(pClientSocket, SIGNAL(disconnected()), pClientSocket, SLOT(deleteLater()));
	connect(pClientSocket, SIGNAL(readyRead()), this, SLOT(SlotReadClient()));
	SendToClient(pClientSocket);
}

void ClientServer::SlotReadClient()
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

void ClientServer::SendHit(const quint8 coord)
{
	if (_currentState != DOIT::MYMOVE)
		return;
	_senderPacket.WriteData(DOIT::HIT, coord);
}

void ClientServer::Disconnect()
{
	_senderPacket.WriteData(DOIT::STOPGAME);
	_server.close();
}
