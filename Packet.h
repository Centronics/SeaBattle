#pragma once
#include <utility>
#include "Ship.h"

class Packet
{
public:

	enum class DOIT : quint8
	{
		PUSHMAP,
		HIT
	};

	enum class STATE : quint8
	{
		NOERR,
		ERR,
		DISCONNECTED,
		CONNECTED
	};

private:

	std::vector<quint8> _massive{ 0 };
	STATE _error = STATE::NOERR;
	QString _errorMessage;

public:

	Packet() = default;
	explicit Packet(QDataStream& data, quint16 blockSize);
	explicit Packet(QString errorMessage) : _error(STATE::ERR), _errorMessage(std::move(errorMessage)) { }
	explicit Packet(const STATE state) : _error(state) { }
	Packet(const Packet&) = default;
	Packet(Packet&& packet) noexcept;
	~Packet() = default;
	Packet& operator=(const Packet&) = delete;
	Packet& operator=(Packet&&) = delete;

	void WriteData(DOIT doit, quint8 param);
	void WriteData(const std::vector<Ship>& mas);
	[[nodiscard]] bool ReadData(DOIT& doit, quint8& param) const;
	[[nodiscard]] bool ReadRivals(std::vector<Ship>& mas) const;
	[[nodiscard]] bool SerializeToQDataStream(QDataStream& data) const;
	[[nodiscard]] STATE GetState(QString* errStr = nullptr) const;

	[[nodiscard]] explicit operator bool() const noexcept
	{
		return _error == STATE::NOERR;
	}
};
