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
	[[nodiscard]] bool WriteToQDataStream(QDataStream& data) const;

	void Clear() noexcept
	{
		_massive.clear();
	}
};
