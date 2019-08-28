#include "stdafx.h"
#include "Client.h"

using namespace std;

Client::Client(Graphics& g, SeaBattle& c, QObject* parent) : NetworkInterface(g, c, parent)
{
	connect(&_tcpSocket, SIGNAL(connected()), SLOT(SlotConnected()));
	connect(&_tcpSocket, SIGNAL(readyRead()), SLOT(SlotReadyRead()));
	connect(&_tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(SlotError(QAbstractSocket::SocketError)));
	connect(&_tcpSocket, SIGNAL(disconnected()), SLOT(SlotClosed()));
	_currentState = STATE::PUSHMAP;
}

inline void Client::SendHit(const quint8 coord)
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
	SendToServer(packet);
}

void Client::Close()
{
	_currentState = STATE::PUSHMAP;
	_tcpSocket.close();
}

void Client::Connect(const QString& ip, const quint16 port)
{
	_tcpSocket.close();
	_tcpSocket.connectToHost(ip, port, QIODevice::ReadWrite, QAbstractSocket::NetworkLayerProtocol::IPv4Protocol);
}

void Client::IncomingProc(Packet packet)
{
	switch (Packet out; _currentState)
	{
	case STATE::PUSHMAP:
		out.WriteData(_graphics.GetData());
		SendToServer(out);
		_currentState = STATE::WAITMAP;
		break;
	case STATE::WAITMAP:
		if (!packet.ReadRivals(_graphics.GetData()))
		{
			Close();
			emit SignalReceive(Packet("WAITMAP error."));
			break;
		}
		_currentState = STATE::WAITHIT;
		emit SignalReceive(Packet(Packet::STATE::CONNECTED));
		break;
	case STATE::WAITHIT:
		quint8 coord;
		if (Packet::DOIT doit; !packet.ReadData(doit, coord) || doit != Packet::DOIT::HIT)
		{
			Close();
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
	IncomingProc(Packet(in, blockSize));
}

void Client::SendToServer(const Packet& packet)
{
	_tcpSocket.write(GetBytes(packet));
}

void Client::SlotConnected()
{
	_currentState = STATE::PUSHMAP;
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
	IncomingProc(Packet(errorString));
}
