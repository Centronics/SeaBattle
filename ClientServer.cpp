#include "stdafx.h"
#include "Packet.h"
#include "ClientServer.h"

using namespace std;

void ClientServer::Send(const Packet& packet)
{
	switch(_currentState)
	{
	case DOIT::PUSHMAP:
		break;
	case DOIT::STARTGAME:
		break;
	case DOIT::STOPGAME:
		break;
	case DOIT::END:
		break;
	case DOIT::HIT:
		break;
	case DOIT::CONNECTIONERROR:
		break;
	case DOIT::CONNECTED:
		break;
	case DOIT::SHIPADDITION:
		break;
	case DOIT::WAITRIVAL:
		break;
	case DOIT::MYMOVE:
		break;
	case DOIT::RIVALMOVE:
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

void ClientServer::Disconnect()
{

}
