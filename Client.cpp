#include "stdafx.h"
#include "Client.h"

using namespace std;

Client::Client(Graphics& g, SeaBattle& c, QObject* parent) : NetworkInterface(g, c, parent)
{
	connect(&_tcpSocket, SIGNAL(connected()), SLOT(SlotConnected()));
	connect(&_tcpSocket, SIGNAL(readyRead()), SLOT(SlotReadyRead()));
	connect(&_tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(SlotError(QAbstractSocket::SocketError)));
}

inline void Client::SendHit(const quint8 coord)
{
	if (_currentState != DOIT::HIT)
		return;
	Packet packet;
	packet.WriteData(DOIT::HIT, coord);
	SendToServer(packet);
}

void Client::Connect(const QString& ip, const quint16 port)
{
	_tcpSocket.close();
	_tcpSocket.connectToHost(ip, port, QIODevice::ReadWrite, QAbstractSocket::NetworkLayerProtocol::IPv4Protocol);
}

void Client::Send(const Packet* packet)
{
	switch (_currentState)
	{
	case DOIT::STARTGAME:
	{
		if (packet != nullptr)
			throw exception(__func__);
		Packet out;
		out.WriteData(DOIT::STARTGAME);
		SendToServer(out);
		_currentState = DOIT::PUSHMAP;
		break;
	}
	case DOIT::PUSHMAP:
	{
		if (packet == nullptr)
			throw exception(__func__);
		DOIT doit;
		if (!packet->ReadData(doit) || doit != DOIT::STARTGAME)
		{
			_tcpSocket.close();
			break;
		}
		Packet out;
		out.WriteData(_graphics.GetData());
		SendToServer(out);
		_currentState = DOIT::WAITMAP;
		break;
	}
	case DOIT::WAITMAP:
	{
		if (packet == nullptr)
			throw exception(__func__);
		DOIT doit;
		if (!packet->ReadData(doit) || doit != DOIT::PUSHMAP)
		{
			_tcpSocket.close();
			break;
		}
		if (!_graphics.ReadEnemies(*packet))
		{
			_currentState = DOIT::STARTGAME;
			_tcpSocket.close();
			return;
		}
		_currentState = DOIT::WAITRIVAL;
		emit Connected(true, "Клиент.", QString());
		break;
	}
	case DOIT::WAITRIVAL:
	{

		break;
	}
	case DOIT::HIT:
	{
		_currentState = DOIT::WAITRIVAL;
		break;
	}
	case DOIT::STOPGAME:
	{
		_tcpSocket.close();
		_currentState = DOIT::STARTGAME;
		break;
	}
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
	Packet packet(in, blockSize);
	Send(&packet);
}

void Client::SendToServer(const Packet& packet)
{
	QByteArray arrBlock;
	QDataStream out(&arrBlock, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_5_10);
	out << quint16(0);
	if (!packet.SerializeToQDataStream(out))
		return;
	out.device()->seek(0);
	out << quint16(arrBlock.size() - 2);
	_tcpSocket.write(arrBlock);
}

void Client::SlotConnected()
{
	Send();
}

void Client::SlotError(const QAbstractSocket::SocketError err)
{
	QString errorString;
	switch (err)
	{
	case QAbstractSocket::ConnectionRefusedError:
		errorString = "The connection was refused by the peer (or timed out).";
		break;
	case QAbstractSocket::RemoteHostClosedError:
		errorString = "The remote host closed the connection. Note that the client socket (i.e., this socket) will be closed after the remote close notification has been sent.";
		break;
	case QAbstractSocket::HostNotFoundError:
		errorString = "The host address was not found.";
		break;
	case QAbstractSocket::SocketAccessError:
		errorString = "The socket operation failed because the application lacked the required privileges.";
		break;
	case QAbstractSocket::SocketResourceError:
		errorString = "The local system ran out of resources (e.g., too many sockets).";
		break;
	case QAbstractSocket::SocketTimeoutError:
		errorString = "The socket operation timed out.";
		break;
	case QAbstractSocket::DatagramTooLargeError:
		errorString = "The datagram was larger than the operating system's limit (which can be as low as 8192 bytes).";
		break;
	case QAbstractSocket::NetworkError:
		errorString = "An error occurred with the network (e.g., the network cable was accidentally plugged out).";
		break;
	case QAbstractSocket::AddressInUseError:
		errorString = "The address specified to QAbstractSocket::bind() is already in use and was set to be exclusive.";
		break;
	case QAbstractSocket::SocketAddressNotAvailableError:
		errorString = "The address specified to QAbstractSocket::bind() does not belong to the host.";
		break;
	case QAbstractSocket::UnsupportedSocketOperationError:
		errorString = "The requested socket operation is not supported by the local operating system (e.g., lack of IPv6 support).";
		break;
	case QAbstractSocket::UnfinishedSocketOperationError:
		errorString = "Used by QAbstractSocketEngine only, The last operation attempted has not finished yet (still in progress in the background).";
		break;
	case QAbstractSocket::ProxyAuthenticationRequiredError:
		errorString = "The socket is using a proxy, and the proxy requires authentication.";
		break;
	case QAbstractSocket::SslHandshakeFailedError:
		errorString = "The SSL/TLS handshake failed, so the connection was closed (only used in QSslSocket).";
		break;
	case QAbstractSocket::ProxyConnectionRefusedError:
		errorString = "Could not contact the proxy server because the connection to that server was denied.";
		break;
	case QAbstractSocket::ProxyConnectionClosedError:
		errorString = "The connection to the proxy server was closed unexpectedly (before the connection to the final peer was established).";
		break;
	case QAbstractSocket::ProxyConnectionTimeoutError:
		errorString = "The connection to the proxy server timed out or the proxy server stopped responding in the authentication phase.";
		break;
	case QAbstractSocket::ProxyNotFoundError:
		errorString = "The proxy address set with setProxy() (or the application proxy) was not found.";
		break;
	case QAbstractSocket::ProxyProtocolError:
		errorString = "The connection negotiation with the proxy server failed, because the response from the proxy server could not be understood.";
		break;
	case QAbstractSocket::OperationError:
		errorString = "An operation was attempted while the socket was in a rotate that did not permit it.";
		break;
	case QAbstractSocket::SslInternalError:
		errorString = "The SSL library being used reported an internal error. This is probably the result of a bad installation or misconfiguration of the library.";
		break;
	case QAbstractSocket::SslInvalidUserDataError:
		errorString = "Invalid data (certificate, key, cypher, etc.) was provided and its use resulted in an error in the SSL library.";
		break;
	case QAbstractSocket::TemporaryError:
		errorString = "A temporary error occurred (e.g., operation would block and socket is non-blocking).";
		break;
	case QAbstractSocket::UnknownSocketError:
		errorString = "An unidentified error occurred.";
		break;
	default:
		errorString = _tcpSocket.errorString();
		break;
	}
	emit Connected(false, "Клиент.", errorString);
}
