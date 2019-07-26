#pragma once
#include "Ship.h"

enum class DOIT : quint8
{
	PUSHMAP,
	STARTGAME,
	STOPGAME,
	HIT,
	CONNECTIONERROR,
	WAITRIVAL,
	MYMOVE,
	INCORRECTMESSAGE
};

class Packet
{
	std::vector<quint8> _massive{ 0 };

public:

	Packet() = default;
	explicit Packet(QDataStream& data, quint16 blockSize);
	Packet(const Packet&) = default;
	Packet(Packet&& packet) noexcept;
	~Packet() = default;
	Packet& operator=(const Packet&) = delete;
	Packet& operator=(Packet&& packet) noexcept;

	void WriteData(DOIT doit, quint8 param);
	void WriteData(const std::vector<Ship>& mas);
	void WriteData(DOIT doit);
	[[nodiscard]] bool ReadData(DOIT& doit, quint8& param) const;
	[[nodiscard]] bool ReadData(std::vector<Ship>& mas) const;
	[[nodiscard]] bool ReadData(DOIT& doit) const;

	[[nodiscard]] bool WriteToQDataStream(QDataStream& data) const
	{
		if (_massive.empty())
			return false;
		const int sz = static_cast<int>(_massive.size());
		const int written = data.writeRawData(reinterpret_cast<const char*>(_massive.data()), sz);
		if (written < 0)
			return false;
		return written == sz;
	}

	void Clear() noexcept
	{
		_massive.clear();
	}
};
