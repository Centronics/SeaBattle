#pragma once

class Ship
{
	friend class Packet;

public:

	enum class SHIPHOLDER : unsigned char
	{
		NIL,
		ME,
		RIVAL,
		BOTH
	};

	enum class BIT : unsigned char
	{
		NIL,
		MYBEAT,
		RIVALBEAT
	};

	enum class SHIPS : unsigned char
	{
		LINKOR,
		CRUISER,
		ESMINEC,
		VEDETTE
	};

	enum class ROTATE : unsigned char
	{
		NIL,
		STARTRIGHT,
		STARTDOWN,
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
		default:
			throw std::exception(__func__);
		}
	}

	[[nodiscard]] SHIPHOLDER GetHolder() const
	{
		return static_cast<SHIPHOLDER>((_currentState & 0xC0u) >> 6u);
	}

	[[nodiscard]] bool GetIsMyHolding() const
	{
		switch (GetHolder())
		{
		case SHIPHOLDER::BOTH:
		case SHIPHOLDER::ME:
			return true;
		case SHIPHOLDER::NIL:
		case SHIPHOLDER::RIVAL:
			return false;
		default:
			throw std::exception(__func__);
		}
	}

	void SetHolder(const SHIPHOLDER holder)
	{
		switch (holder)
		{
		case SHIPHOLDER::NIL:
			_currentState &= 0x3Fu;
			return;
		case SHIPHOLDER::ME:
			_currentState &= 0x7Fu;
			_currentState |= 0x40u;
			return;
		case SHIPHOLDER::RIVAL:
			_currentState |= 0x80u;
			_currentState &= 0xBFu;
			return;
		case SHIPHOLDER::BOTH:
			_currentState |= 0xC0u;
			return;
		default:
			throw std::exception(__func__);
		}
	}

	[[nodiscard]] BIT GetBit() const
	{
		const quint8 t = (_currentState & 0x30u) >> 4u;
		if (t > 2)
			throw std::exception(__func__);
		return static_cast<BIT>(t);
	}

	void SetBit(const BIT value)
	{
		switch (value)
		{
		case BIT::NIL:
			_currentState &= 0xCFu;
			return;
		case BIT::MYBEAT:
			_currentState &= 0xDFu;
			_currentState |= 0x10u;
			return;
		case BIT::RIVALBEAT:
			_currentState |= 0x20u;
			_currentState &= 0xEFu;
			return;
		default:
			throw std::exception(__func__);
		}
	}

	[[nodiscard]] SHIPS GetShip() const
	{
		return static_cast<SHIPS>((_currentState & 0x0Cu) >> 2u);
	}

	void SetShip(const SHIPS value)
	{
		switch (value)
		{
		case SHIPS::LINKOR:
			_currentState &= 0xF3u;
			return;
		case SHIPS::CRUISER:
			_currentState &= 0xF7u;
			_currentState |= 0x04u;
			return;
		case SHIPS::ESMINEC:
			_currentState |= 0x08u;
			_currentState &= 0xFBu;
			return;
		case SHIPS::VEDETTE:
			_currentState |= 0x0Cu;
			return;
		default:
			throw std::exception(__func__);
		}
	}

	[[nodiscard]] ROTATE GetRotate() const
	{
		const quint8 t = _currentState & 0x03u;
		if (t > 2)
			throw std::exception(__func__);
		return static_cast<ROTATE>(t);
	}

	void SetRotate(const ROTATE value)
	{
		switch (value)
		{
		case ROTATE::NIL:
			_currentState &= 0xFCu;
			return;
		case ROTATE::STARTRIGHT:
			_currentState &= 0xFDu;
			_currentState |= 0x01u;
			return;
		case ROTATE::STARTDOWN:
			_currentState |= 0x02u;
			_currentState &= 0xFEu;
			return;
		default:
			throw std::exception(__func__);
		}
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