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
		_errorMessage = "ReadPacket error(1)";
		return;
	}
	quint16 blockSize = 0;
	data >> blockSize;
	if (socket.bytesAvailable() != blockSize)
	{
		_error = STATE::ERR;
		_errorMessage = "ReadPacket error(2)";
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
			_errorMessage = "ReadPacket error(3)";
			return;
		}
		_massive.resize(101);
		_massive[0] = static_cast<quint8>(DOIT::PUSHMAP);
		if (data.readRawData(reinterpret_cast<char*>(&_massive[1]), 100) == 100)
			return;
		_massive.clear();
		_error = STATE::ERR;
		_errorMessage = "ReadPacket error(4)";
		return;
	case DOIT::HIT:
		if (blockSize != 2)
		{
			_error = STATE::ERR;
			_errorMessage = "ReadPacket error(5)";
			return;
		}
		_massive.resize(2);
		_massive[0] = static_cast<quint8>(DOIT::HIT);
		data >> _massive[1];
		return;
	default:
		throw exception(__func__);
	}
}

Packet::Packet(Packet&& packet) noexcept
{
	_massive = move(packet._massive);
	_error = packet._error;
	_errorMessage = move(packet._errorMessage);
}

bool Packet::SerializeToQDataStream(QDataStream& data) const
{
	if (_massive.empty())
		return false;
	const int sz = _massive.size();
	return data.writeRawData(reinterpret_cast<const char*>(_massive.data()), sz) == sz;
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
	default:
		break;
	}
	return _error;
}

void Packet::WriteData(const DOIT doit, const quint8 param)
{
	if (doit != DOIT::HIT)
		throw exception(__func__);
	_massive.clear();
	_massive.reserve(2);
	_massive.emplace_back(static_cast<quint8>(doit));
	_massive.emplace_back(param);
}

void Packet::WriteData(const vector<Ship>& mas)
{
	if (mas.size() != 100)
		throw exception("Неправильная длина пакета.");
	_massive.clear();
	_massive.reserve(101);
	_massive.emplace_back(static_cast<quint8>(DOIT::PUSHMAP));
	for (const auto& t : mas)
		_massive.emplace_back(t);
}

bool Packet::ReadData(DOIT& doit, quint8& param) const
{
	if (_massive.size() != 2)
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
	if (mas.size() != 100 || _massive.size() != 101 || static_cast<DOIT>(_massive[0]) != DOIT::PUSHMAP)
		return false;
	for (unsigned int k = 0, n = 1; k < 100; ++k, ++n)
	{
		Ship& to = mas[k];
		to.SetBit(Ship::BIT::NIL);
		if (Ship(_massive[n]).GetShipHolder() == Ship::HOLDER::ME)
			to.SetRivalHolding();
		else
			to.ClearRivalHolding();
	}
	return true;
}
