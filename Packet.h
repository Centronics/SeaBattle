#pragma once
#include "Ship.h"

enum class DOIT : unsigned char
{
	PUSHMAP,
	STARTGAME,
	STOPGAME,
	HIT,
	CONNECTIONERROR,
	WAITRIVAL,
	MYMOVE
};

class Packet
{
	friend class ClientServer;

protected:

	std::vector<quint8> _massive{ 0 };
	bool _isCorrect = true;

	explicit Packet(const int elements) : _massive(elements) { }

public:

	Packet() = default;

	explicit Packet(const bool isCorrect) : _isCorrect(isCorrect) { }

	~Packet() = default;
	Packet(const Packet&) = default;
	Packet(Packet&& packet) noexcept;
	Packet& operator=(const Packet&) = delete;
	Packet& operator=(Packet&& packet) noexcept;

	void WriteData(DOIT doit, quint8 param);
	void WriteData(const std::vector<Ship>& mas);
	void WriteData(DOIT doit);
	[[nodiscard]] bool ReadData(DOIT& doit, quint8& param) const;
	[[nodiscard]] bool ReadData(std::vector<Ship>& mas) const;
	[[nodiscard]] bool ReadData(DOIT& doit) const;

	[[nodiscard]] bool IsCorrect() const noexcept
	{
		return _isCorrect;
	}
};
