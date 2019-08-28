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
		DISCONNECTED
	};

private:

	std::vector<quint8> _massive{ 0 };
	STATE _error = STATE::NOERR;
	QString _errorMessage;

public:

	Packet() = default;
	explicit Packet(QDataStream& data, quint16 blockSize);
	explicit Packet(QString errorMessage) : _error(STATE::ERR), _errorMessage(std::move(errorMessage)) { }
	Packet(const Packet&) = delete;
	Packet(Packet&& packet) noexcept;
	~Packet() = default;
	Packet& operator=(const Packet&) = delete;
	Packet& operator=(Packet&&) = delete;

	void WriteData(DOIT doit, quint8 param);
	void WriteData(const std::vector<Ship>& mas);
	[[nodiscard]] bool ReadData(DOIT& doit, quint8& param) const;
	[[nodiscard]] bool ReadRivals(std::vector<Ship>& mas) const;
	[[nodiscard]] bool SerializeToQDataStream(QDataStream& data) const;

	void SetDisconnected() noexcept
	{
		_error = STATE::DISCONNECTED;
	}

	[[nodiscard]] STATE GetState(QString* const errStr = nullptr) const
	{
		if (errStr)
			*errStr = (_error == STATE::DISCONNECTED) ? "Disconnected" : _errorMessage;
		return _error;
	}

	[[nodiscard]] explicit operator bool() const noexcept
	{
		return _error == STATE::NOERR;
	}
};
