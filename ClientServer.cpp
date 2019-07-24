#include "stdafx.h"
#include "ClientServer.h"

using namespace std;

void ClientServer::Send()
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
		Connect();
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

void ClientServer::AddToQueue(const Packet& packet)
{
	lock_guard<std::recursive_mutex> locker(_lock);

}

void ClientServer::SendToClient(QTcpSocket* socket)
{

}

bool ClientServer::StartServer(const int port)
{
	return true;
}

bool ClientServer::StartClient(const QString& ip, int port)
{
	return true;
}

bool ClientServer::Listen(const int port)
{
	if (!_server.listen(QHostAddress::Any, port))
		return false;
	connect(&_server, SIGNAL(newConnection()), this, SLOT(SlotNewConnection()));
	return true;
}

Packet ClientServer::GetFromQueue() const
{
	lock_guard<recursive_mutex> locker(_lock);
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
	quint8 doit;
	in >> doit;
	lock_guard<recursive_mutex> locker(_lock);
	switch (const DOIT dt = static_cast<DOIT>(doit)) // œ≈–≈Õ≈—“» ¬ PAKCET Ë Â‡ÎËÁÓ‚‡Ú¸ Ò‚ÓÈÒÚ‚Ó IsCorrect.
	{
	case DOIT::PUSHMAP:
	{
		Packet packet(101);
		quint8* ptr = packet._massive.data();
		*ptr = static_cast<quint8>(DOIT::PUSHMAP);
		quint8* ptrMain = &ptr[1];
		for (quint16 k = 0; k < blockSize; k++, ptrMain++)
			in >> *ptrMain;
		break;
	}
	case DOIT::STARTGAME:
	case DOIT::STOPGAME:
	case DOIT::CONNECTIONERROR:
	case DOIT::WAITRIVAL:
	case DOIT::MYMOVE:
	{
		Packet packet(1);
		*(packet._massive.data()) = static_cast<quint8>(dt);
		break;
	}
	case DOIT::HIT:
	{
		Packet packet(2);
		quint8* p = packet._massive.data();
		*p++ = static_cast<quint8>(dt);
		in >> *p;
		break;
	}
	default:
		return;
	}
}

bool ClientServer::Connect()
{
	return true;
}

void ClientServer::SendHit(const quint8 coord)
{
	if (_currentState != DOIT::MYMOVE)
		return;
	_packet.WriteData(DOIT::HIT, coord);
}

void ClientServer::Disconnect()
{
	_packet.WriteData(DOIT::STOPGAME);
}
