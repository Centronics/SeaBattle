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
	case DOIT::STOPGAME:
	case DOIT::WAITRIVAL:
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

bool Packet::WriteToQDataStream(QDataStream& data) const
{
	if (_massive.empty())
		return false;
	const int sz = static_cast<int>(_massive.size());
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
	case DOIT::STOPGAME:
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
		return;
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

bool Packet::ReadData(vector<Ship>& mas) const
{
	if (_massive.size() != 101 || static_cast<DOIT>(_massive[0]) != DOIT::PUSHMAP)
		return false;
	mas.clear();
	mas.reserve(100);
	for (unsigned int k = 1; k < 101; ++k)
		mas.emplace_back(Ship(_massive[k]));
	return true;
}

bool Packet::ReadData(DOIT& doit) const
{
	if (_massive.size() != 1)
		return false;
	switch (doit = static_cast<DOIT>(_massive[0]))
	{
	case DOIT::STARTGAME:
	case DOIT::STOPGAME:
		doit = static_cast<DOIT>(_massive[1]);
		return true;
	default: return false;
	}
}

bool Packet::ReadEnemies(std::vector<Ship>& mas) const
{
	if (mas.size() != 100 || _massive.size() != 101 || static_cast<DOIT>(_massive[0]) != DOIT::PUSHMAP)
		return false;
	for (unsigned int k = 0, n = 1; k < 100; ++k, ++n)
	{
		Ship ship = Ship(_massive[n]);
		if (ship.GetHolder() == Ship::SHIPHOLDER::ME)
			mas[k].SetHolder(Ship::SHIPHOLDER::RIVAL);
	}
	return true;
}
