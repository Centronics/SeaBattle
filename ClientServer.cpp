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

bool ClientServer::StartServer(const int port)
{
	return true;
}

bool ClientServer::StartClient(const QString& ip, int port)
{
	return true;
}

bool ClientServer::Connect()
{
	return true;
}

void ClientServer::SendHit(const quint8 coord)
{
	if(_currentState != DOIT::MYMOVE)
		return;
	_packet.WriteData(DOIT::HIT, coord);
}

void ClientServer::Disconnect()
{
	_packet.WriteData(DOIT::STOPGAME);
}
