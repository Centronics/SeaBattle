#include "stdafx.h"
#include "Packet.h"

using namespace std;

Packet::Packet(QTcpSocket& socket)
{
	QDataStream data(&socket);
	data.setVersion(QDataStream::Qt_5_10);
	if (socket.bytesAvailable() < 2)
	{
		_error = STATE::ERR;
		_errorMessage = "Packet error(1)";
		return;
	}
	quint16 blockSize = 0;
	data >> blockSize;
	if (socket.bytesAvailable() != blockSize)
	{
		_error = STATE::ERR;
		_errorMessage = "Packet error(2)";
		return;
	}
	quint8 doit;
	data >> doit;
	switch (static_cast<DOIT>(doit))
	{
	case DOIT::PUSHMAP:
		if (blockSize != 101)
		{
			_error = STATE::ERR;
			_errorMessage = "Packet error(3)";
			return;
		}
		_massive.resize(101);
		_massive[0] = static_cast<quint8>(DOIT::PUSHMAP);
		if (data.readRawData(reinterpret_cast<char*>(&_massive[1]), 100) == 100)
			return;
		_massive.clear();
		_error = STATE::ERR;
		_errorMessage = "Packet error(4)";
		return;
	case DOIT::HIT:
		if (blockSize != 2)
		{
			_error = STATE::ERR;
			_errorMessage = "Packet error(5)";
			return;
		}
		_massive.resize(2);
		_massive[0] = static_cast<quint8>(DOIT::HIT);
		data >> _massive[1];
		return;
	case DOIT::BUSY:
		if (blockSize != 1)
		{
			_error = STATE::ERR;
			_errorMessage = "Packet error(6)";
			return;
		}
		_error = STATE::BUSY;
		return;
	default:
		throw exception(__func__);
	}
}

void Packet::MoveFunc(Packet&& packet)
{
	_massive = move(packet._massive);
	_error = packet._error;
	_errorMessage = move(packet._errorMessage);
}

Packet::Packet(Packet&& packet) noexcept
{
	MoveFunc(move(packet));
}

Packet& Packet::operator=(Packet&& packet) noexcept
{
	MoveFunc(move(packet));
	return *this;
}

bool Packet::SerializeToQDataStream(QDataStream& data) const
{
	if (_error == STATE::NOERR && !_massive.empty())
	{
		const int sz = _massive.size();
		return data.writeRawData(reinterpret_cast<const char*>(_massive.data()), sz) == sz;
	}
	if (_error == STATE::BUSY && _massive.empty())
	{
		const DOIT dt = DOIT::BUSY;
		return data.writeRawData(reinterpret_cast<const char*>(&dt), 1) == 1;
	}
	return false;
}

Packet::STATE Packet::GetState(QString* const errStr) const
{
	switch (_error)
	{
	case STATE::ERR:
		if (errStr)
			*errStr = _errorMessage;
		break;
	case STATE::DISCONNECTED:
		if (errStr)
			*errStr = "Disconnected";
		break;
	case STATE::CONNECTED:
		if (errStr)
			*errStr = "Connected";
		break;
	case STATE::BUSY:
		if (errStr)
			*errStr = "Busy";
		break;
	default:
		break;
	}
	return _error;
}

void Packet::WriteData(const DOIT doit, const quint8 param)
{
	if (_error != STATE::NOERR || doit != DOIT::HIT)
		throw exception(__func__);
	_massive.clear();
	_massive.reserve(2);
	_massive.emplace_back(static_cast<quint8>(doit));
	_massive.emplace_back(param);
}

void Packet::WriteData(const vector<Ship>& mas)
{
	if (_error != STATE::NOERR || mas.size() != 100)
		throw exception(__func__);
	_massive.clear();
	_massive.reserve(101);
	_massive.emplace_back(static_cast<quint8>(DOIT::PUSHMAP));
	for (const auto& t : mas)
		_massive.emplace_back(t);
}

bool Packet::ReadData(DOIT& doit, quint8& param) const
{
	if (_error != STATE::NOERR || _massive.size() != 2)
		return false;
	if (const DOIT dt = static_cast<DOIT>(_massive[0]); dt == DOIT::HIT)
	{
		doit = dt;
		param = _massive[1];
		return true;
	}
	return false;
}

bool Packet::ReadRivals(std::vector<Ship>& mas) const
{
	if (_error != STATE::NOERR || mas.size() != 100 || _massive.size() != 101 || static_cast<DOIT>(_massive[0]) != DOIT::PUSHMAP)
		return false;
	for (unsigned int k = 0, n = 1; k < 100; ++k, ++n)
	{
		Ship& to = mas[k];
		to.SetBit(Ship::BIT::NIL);
		if (Ship(_massive[n]).GetHolding(Ship::HOLDING::ME))
			to.SetRivalHolding();
		else
			to.ClearRivalHolding();
	}
	return true;
}

void Packet::Send(QTcpSocket& pSocket) const
{
	if (_error != STATE::NOERR && _error != STATE::BUSY)
		throw exception(__func__);
	QByteArray arrBlock;
	QDataStream out(&arrBlock, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_5_10);
	out << quint16(0);
	if (!SerializeToQDataStream(out))
		throw std::exception("Не могу сериализовать пакет.");
	out.device()->seek(0);
	out << quint16(arrBlock.size() - 2);
	pSocket.write(arrBlock);
}