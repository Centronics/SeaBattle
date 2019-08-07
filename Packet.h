#pragma once
#include <utility>
#include "Ship.h"

enum class DOIT : quint8
{
	STARTGAME,
	PUSHMAP,
	WAITMAP,
	HIT,
	WAITHIT,
	STOPGAME
};

class Packet
{
	std::vector<quint8> _massive{ 0 };
	bool _error = false;
	QString _errorMessage;

public:

	Packet() = default;
	explicit Packet(QDataStream& data, quint16 blockSize);
	explicit Packet(QString errorMessage) : _error(true), _errorMessage(std::move(errorMessage)) { }
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
	[[nodiscard]] bool ReadRivals(std::vector<Ship>& mas) const;
	[[nodiscard]] bool SerializeToQDataStream(QDataStream& data) const;

	[[nodiscard]] const QString& ErrorString() const
	{
		return _errorMessage;
	}

	[[nodiscard]] explicit operator bool() const
	{
		return !_error;
	}

	void Clear() noexcept
	{
		_massive.clear();
	}
};
