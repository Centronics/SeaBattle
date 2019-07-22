#pragma once

class Ship
{
	friend class Packet;

public:

	enum class SHIPHOLDER : unsigned char
	{
		ME,
		RIVAL
	};

	enum class SHIPS : unsigned char
	{
		EMPTY,
		LINKOR,
		CRUISER,
		ESMINEC,
		VEDETTE
	};

	enum class STATE : unsigned char
	{
		NIL,
		STARTRIGHT,
		STARTDOWN,
	};

	enum class BIT : unsigned char
	{
		NIL,
		MYBEAT,
		RIVALBEAT
	};

	Ship() = default;
	Ship(const Ship&) = delete;
	Ship(Ship&&) = default;
	~Ship() = default;
	Ship& operator=(const Ship&) = delete;
	Ship& operator=(Ship&&) = default;

protected:

	explicit Ship(const quint8 byte)
	{
		_currentState = byte;
	}

public:

	[[nodiscard]] static int GetFloors(const SHIPS ship)
	{
		switch (ship)
		{
		case SHIPS::LINKOR: return 4;
		case SHIPS::CRUISER: return 3;
		case SHIPS::ESMINEC: return 2;
		case SHIPS::VEDETTE: return 1;
		case SHIPS::EMPTY:
		default:
			throw std::exception(__func__);
		}
	}

	[[nodiscard]] static QColor GetColor(const SHIPS ship)
	{
		switch (ship)
		{
		case SHIPS::LINKOR: return Qt::darkMagenta;
		case SHIPS::CRUISER: return Qt::green;
		case SHIPS::ESMINEC: return Qt::darkYellow;
		case SHIPS::VEDETTE: return Qt::cyan;
		case SHIPS::EMPTY:
		default:
			throw std::exception(__func__);
		}
	}

	[[nodiscard]] static int GetMaxShipCount(const SHIPS ship)
	{
		switch (ship)
		{
		case SHIPS::LINKOR: return 1;
		case SHIPS::CRUISER: return 2;
		case SHIPS::ESMINEC: return 3;
		case SHIPS::VEDETTE: return 4;
		case SHIPS::EMPTY:
		default:
			throw std::exception(__func__);
		}
	}

	[[nodiscard]] SHIPHOLDER GetHolder() const
	{
		return (_currentState & 0x80u) == 0x80u ? SHIPHOLDER::RIVAL : SHIPHOLDER::ME;
	}

	void SetHolder(const SHIPHOLDER holder)
	{
		switch (holder)
		{
		case SHIPHOLDER::ME:
			_currentState &= 0x7Fu;
			return;
		case SHIPHOLDER::RIVAL:
			_currentState |= 0x80u;
			return;
		default:
			throw std::exception(__func__);
		}
	}

	void SetBit(const BIT value)
	{
		_currentState &= static_cast<quint8>(value) << 2;
	}

	[[nodiscard]] BIT GetBit() const
	{
		return static_cast<BIT>((_currentState & 0x0Cu) >> 2);
	}

	void SetShip(const SHIPS value)
	{
		_currentState = (_currentState & 0x0Fu) | (static_cast<quint8>(value) << 4);
	}

	[[nodiscard]] SHIPS GetShip() const
	{
		return static_cast<SHIPS>((_currentState & 0xF0u) >> 4);
	}

	void SetState(const STATE value)
	{
		_currentState = (_currentState & 0xFCu) | static_cast<quint8>(value);
	}

	[[nodiscard]] STATE GetState() const
	{
		return static_cast<STATE>(_currentState & 0x03u);
	}

	void Delete()
	{
		_currentState = quint8{};
	}

	explicit operator quint8() const
	{
		return _currentState;
	}

private:

	quint8 _currentState{ 0 };
};