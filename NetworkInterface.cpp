#include "stdafx.h"
#include "NetworkInterface.h"

QByteArray NetworkInterface::GetBytes(const Packet& packet)
{
	QByteArray arrBlock;
	QDataStream out(&arrBlock, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_5_10);
	out << quint16(0);
	if (!packet.SerializeToQDataStream(out))
		throw std::exception("Не могу сериализовать пакет.");
	out.device()->seek(0);
	out << quint16(arrBlock.size() - 2);
	return arrBlock;
}

QString NetworkInterface::GetErrorDescr(const QAbstractSocket::SocketError err, const QString& alternate)
{
	switch (err)
	{
	case QAbstractSocket::ConnectionRefusedError:
		return "The connection was refused by the peer (or timed out).";
	case QAbstractSocket::RemoteHostClosedError:
		return "The remote host closed the connection. Note that the client socket (i.e., this socket) will be closed after the remote close notification has been sent.";
	case QAbstractSocket::HostNotFoundError:
		return "The host address was not found.";
	case QAbstractSocket::SocketAccessError:
		return "The socket operation failed because the application lacked the required privileges.";
	case QAbstractSocket::SocketResourceError:
		return "The local system ran out of resources (e.g., too many sockets).";
	case QAbstractSocket::SocketTimeoutError:
		return "The socket operation timed out.";
	case QAbstractSocket::DatagramTooLargeError:
		return "The datagram was larger than the operating system's limit (which can be as low as 8192 bytes).";
	case QAbstractSocket::NetworkError:
		return "An error occurred with the network (e.g., the network cable was accidentally plugged out).";
	case QAbstractSocket::AddressInUseError:
		return "The address specified to QAbstractSocket::bind() is already in use and was set to be exclusive.";
	case QAbstractSocket::SocketAddressNotAvailableError:
		return "The address specified to QAbstractSocket::bind() does not belong to the host.";
	case QAbstractSocket::UnsupportedSocketOperationError:
		return "The requested socket operation is not supported by the local operating system (e.g., lack of IPv6 support).";
	case QAbstractSocket::UnfinishedSocketOperationError:
		return "Used by QAbstractSocketEngine only, The last operation attempted has not finished yet (still in progress in the background).";
	case QAbstractSocket::ProxyAuthenticationRequiredError:
		return "The socket is using a proxy, and the proxy requires authentication.";
	case QAbstractSocket::SslHandshakeFailedError:
		return "The SSL/TLS handshake failed, so the connection was closed (only used in QSslSocket).";
	case QAbstractSocket::ProxyConnectionRefusedError:
		return "Could not contact the proxy server because the connection to that server was denied.";
	case QAbstractSocket::ProxyConnectionClosedError:
		return "The connection to the proxy server was closed unexpectedly (before the connection to the final peer was established).";
	case QAbstractSocket::ProxyConnectionTimeoutError:
		return "The connection to the proxy server timed out or the proxy server stopped responding in the authentication phase.";
	case QAbstractSocket::ProxyNotFoundError:
		return "The proxy address set with setProxy() (or the application proxy) was not found.";
	case QAbstractSocket::ProxyProtocolError:
		return "The connection negotiation with the proxy server failed, because the response from the proxy server could not be understood.";
	case QAbstractSocket::OperationError:
		return "An operation was attempted while the socket was in a rotate that did not permit it.";
	case QAbstractSocket::SslInternalError:
		return "The SSL library being used reported an internal error. This is probably the result of a bad installation or misconfiguration of the library.";
	case QAbstractSocket::SslInvalidUserDataError:
		return "Invalid data (certificate, key, cypher, etc.) was provided and its use resulted in an error in the SSL library.";
	case QAbstractSocket::TemporaryError:
		return "A temporary error occurred (e.g., operation would block and socket is non-blocking).";
	case QAbstractSocket::UnknownSocketError:
		return "An unidentified error occurred.";
	default:
		return alternate;
	}
}

Packet NetworkInterface::CreateHitPacket()
{
	const auto coord = _graphics.GetCoord();
	if (_currentState != STATE::HIT || !coord)
		return Packet(Packet::STATE::ERR);
	if (!_graphics.MyHit(*coord))
	{
		_currentState = STATE::WAITHIT;
		Graphics::IsRivalMove = true;
	}
	Packet packet;
	packet.WriteData(Packet::DOIT::HIT, *coord);
	return packet;
}
