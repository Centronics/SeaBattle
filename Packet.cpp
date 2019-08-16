#include "stdafx.h"
#include "Packet.h"

using namespace std;

Packet::Packet(QDataStream& data, const quint16 blockSize) : _massive(blockSize)
{
	quint8 doit;
	data >> doit;
	switch (const DOIT dt = static_cast<DOIT>(doit))
	{
	case DOIT::PUSHMAP:
	{
		if (blockSize != 101)
			return;
		_massive.resize(101);
		if (data.readRawData(reinterpret_cast<char*>(_massive.data()), 101) != 101)
			_massive.clear();
		return;
	}
	case DOIT::STARTGAME:
	case DOIT::WAITHIT:
		if (blockSize != 1)
			return;
		_massive.resize(1);
		_massive[0] = static_cast<quint8>(dt);
		return;
	case DOIT::HIT:
		if (blockSize != 2)
			return;
		_massive.resize(2);
		_massive[0] = static_cast<quint8>(dt);
		data >> _massive[1];
		return;
	default:
		return;
	}
}

bool Packet::SerializeToQDataStream(QDataStream& data) const
{
	if (_massive.empty())
		return false;
	const int sz = _massive.size();
	return data.writeRawData(reinterpret_cast<const char*>(_massive.data()), sz) == sz;
}

Packet::Packet(Packet&& packet) noexcept
{
	_massive = move(packet._massive);
}

Packet& Packet::operator=(Packet&& packet) noexcept
{
	_massive = move(packet._massive);
	return *this;
}

void Packet::WriteData(const DOIT doit, const quint8 param)
{
	switch (doit)
	{
	case DOIT::STARTGAME:
	case DOIT::WAITMAP:
	case DOIT::STOPGAME:
	case DOIT::WAITHIT:
	case DOIT::PUSHMAP:
		return;
	default:
		break;
	}
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

void Packet::WriteData(const DOIT doit)
{
	switch (doit)
	{
	case DOIT::HIT:
	case DOIT::PUSHMAP:
	case DOIT::WAITHIT:
	case DOIT::WAITMAP:
		return;
	default:
		break;
	}
	_massive.clear();
	_massive.reserve(1);
	_massive.emplace_back(static_cast<quint8>(doit));
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

bool Packet::ReadData(DOIT& doit) const
{
	if (_massive.size() != 1)
		return false;
	switch (const DOIT dt = static_cast<DOIT>(_massive[0]))
	{
	case DOIT::STARTGAME:
	case DOIT::STOPGAME:
		doit = dt;
		return true;
	default:
		return false;
	}
}

bool Packet::ReadRivals(std::vector<Ship>& mas) const
{
	if (mas.size() != 100 || _massive.size() != 101 || static_cast<DOIT>(_massive[0]) != DOIT::PUSHMAP)
		return false;
	for (unsigned int k = 0, n = 1; k < 100; ++k, ++n)
		if (Ship& to = mas[k]; Ship(_massive[n]).GetShipHolder() == Ship::HOLDER::ME)
			if (to.GetShipHolder() == Ship::HOLDER::ME)
				to.SetShipHolder(Ship::HOLDER::BOTH);
			else
				if (to.GetShipHolder() == Ship::HOLDER::NIL)
					to.SetShipHolder(Ship::HOLDER::RIVAL);
	return true;
}
