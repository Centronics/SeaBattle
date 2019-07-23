#include "stdafx.h"
#include "Packet.h"

using namespace std;

Packet::Packet(Packet&& packet) noexcept
{
	_isCorrect = packet._isCorrect;
	_massive = move(packet._massive);
}

Packet& Packet::operator=(Packet&& packet) noexcept
{
	_isCorrect = packet._isCorrect;
	_massive = move(packet._massive);
	return *this;
}

void Packet::WriteData(const DOIT doit, const quint8 param)
{
	if (!_isCorrect)
		return;
	switch (doit)
	{
	case DOIT::END:
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

void Packet::WriteData(const std::vector<Ship>& mas)
{
	if (!_isCorrect || mas.size() != 100)
		return;
	_massive.clear();
	_massive.reserve(101);
	_massive.emplace_back(static_cast<quint8>(DOIT::PUSHMAP));
	for (const auto& t : mas)
		_massive.emplace_back(t);
}

void Packet::WriteData(const DOIT doit)
{
	if (!_isCorrect)
		return;
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
	if (!_isCorrect || _massive.size() != 2)
		return false;
	if (const DOIT dt = static_cast<DOIT>(_massive[0]); dt == DOIT::HIT)
	{
		doit = dt;
		param = _massive[1];
		return true;
	}
	return false;
}

bool Packet::ReadData(std::vector<Ship>& mas) const
{
	if (!_isCorrect || _massive.size() != 101 || static_cast<DOIT>(_massive[0]) != DOIT::PUSHMAP)
		return false;
	mas.clear();
	mas.reserve(100);
	for (unsigned int k = 1; k < 101; ++k)
		mas.emplace_back(Ship(_massive[k]));
	return true;
}

bool Packet::ReadData(DOIT& doit) const
{
	if (!_isCorrect || _massive.size() != 1)
		return false;
	switch (doit = static_cast<DOIT>(_massive[0]))
	{
	case DOIT::END:
	case DOIT::STARTGAME:
	case DOIT::STOPGAME:
		doit = static_cast<DOIT>(_massive[1]);
		return true;
	default: return false;
	}
}
