#pragma once

class Ship
{
	friend class Packet;

public:

	enum class BEAT
	{
		ME,
		RIVAL
	};

	enum class HOLDING
	{
		ME,
		RIVAL
	};

	enum class HOLDER : unsigned char
	{
		NIL,
		ME,
		RIVAL,
		BOTH
	};

	enum class BIT : unsigned char
	{
		NIL,
		ME,
		RIVAL,
		BOTH
	};

	enum class TYPES : unsigned char
	{
		LINKOR,
		CRUISER,
		ESMINEC,
		VEDETTE,
		EMPTY
	};

	enum class ROTATE : unsigned char
	{
		NIL,
		STARTRIGHT,
		STARTDOWN
	};

	Ship() = default;
	Ship(const Ship&) = delete;
	Ship(Ship&&) = delete;
	~Ship() = default;
	Ship& operator=(const Ship&) = delete;
	Ship& operator=(Ship&&) = delete;

protected:

	explicit Ship(const quint8 byte) noexcept
	{
		_currentState = byte;
	}

public:

	[[nodiscard]] static int GetFloors(const TYPES ship)
	{
		switch (ship)
		{
		case TYPES::LINKOR: return 4;
		case TYPES::CRUISER: return 3;
		case TYPES::ESMINEC: return 2;
		case TYPES::VEDETTE: return 1;
		case TYPES::EMPTY: return 0;
		default:
			throw std::exception(__func__);
		}
	}

	[[nodiscard]] static QColor GetColor(const TYPES ship)
	{
		switch (ship)
		{
		case TYPES::LINKOR: return Qt::darkMagenta;
		case TYPES::CRUISER: return Qt::green;
		case TYPES::ESMINEC: return QColor(255, 165, 0);
		case TYPES::VEDETTE: return Qt::cyan;
		default:
			throw std::exception(__func__);
		}
	}

	[[nodiscard]] static int GetMaxShipCount(const TYPES ship)
	{
		switch (ship)
		{
		case TYPES::LINKOR: return 1;
		case TYPES::CRUISER: return 2;
		case TYPES::ESMINEC: return 3;
		case TYPES::VEDETTE: return 4;
		default:
			throw std::exception(__func__);
		}
	}

	[[nodiscard]] HOLDER GetShipHolder() const noexcept
	{
		return static_cast<HOLDER>((_currentState & 0xC0u) >> 6u);
	}

	[[nodiscard]] bool GetHolding(const HOLDING holding) const
	{
		const HOLDER h = GetShipHolder();
		if ((h == HOLDER::BOTH || h == HOLDER::ME) && holding == HOLDING::ME)
			return true;
		if ((h == HOLDER::BOTH || h == HOLDER::RIVAL) && holding == HOLDING::RIVAL)
			return true;
		return false;
	}

	void ClearRivalHolding()
	{
		switch (GetShipHolder())
		{
		case HOLDER::NIL:
		case HOLDER::ME:
			return;
		case HOLDER::RIVAL:
			SetShipHolder(HOLDER::NIL);
			return;
		case HOLDER::BOTH:
			SetShipHolder(HOLDER::ME);
			return;
		default:
			throw std::exception(__func__);
		}
	}

	void SetRivalHolding()
	{
		switch (GetShipHolder())
		{
		case HOLDER::NIL:
			SetShipHolder(HOLDER::RIVAL);
			return;
		case HOLDER::ME:
			SetShipHolder(HOLDER::BOTH);
			return;
		case HOLDER::RIVAL:
		case HOLDER::BOTH:
			return;
		default:
			throw std::exception(__func__);
		}
	}

	void SetShipHolder(const HOLDER holder)
	{
		switch (holder)
		{
		case HOLDER::NIL:
			_currentState &= 0x3Fu;
			return;
		case HOLDER::ME:
			_currentState &= 0x7Fu;
			_currentState |= 0x40u;
			return;
		case HOLDER::RIVAL:
			_currentState |= 0x80u;
			_currentState &= 0xBFu;
			return;
		case HOLDER::BOTH:
			_currentState |= 0xC0u;
			return;
		default:
			throw std::exception(__func__);
		}
	}

	[[nodiscard]] BIT GetBit() const noexcept
	{
		return static_cast<BIT>((_currentState & 0x30u) >> 4u);
	}

	[[nodiscard]] bool GetBeat(const BEAT beat) const
	{
		const BIT b = GetBit();
		if ((b == BIT::BOTH || b == BIT::ME) && beat == BEAT::ME)
			return true;
		if ((b == BIT::BOTH || b == BIT::RIVAL) && beat == BEAT::RIVAL)
			return true;
		return false;
	}

	void SetBit(const BIT value)
	{
		switch (value)
		{
		case BIT::NIL:
			_currentState &= 0xCFu;
			return;
		case BIT::ME:
			_currentState &= 0xDFu;
			_currentState |= 0x10u;
			return;
		case BIT::RIVAL:
			_currentState |= 0x20u;
			_currentState &= 0xEFu;
			return;
		case BIT::BOTH:
			_currentState |= 0x30u;
			return;
		default:
			throw std::exception(__func__);
		}
	}

	[[nodiscard]] TYPES GetShipType() const
	{
		if (GetShipHolder() == HOLDER::NIL)
			return TYPES::EMPTY;
		return static_cast<TYPES>((_currentState & 0x0Cu) >> 2u);
	}

	void SetShipType(const TYPES value)
	{
		switch (value)
		{
		case TYPES::LINKOR:
			_currentState &= 0xF3u;
			return;
		case TYPES::CRUISER:
			_currentState &= 0xF7u;
			_currentState |= 0x04u;
			return;
		case TYPES::ESMINEC:
			_currentState |= 0x08u;
			_currentState &= 0xFBu;
			return;
		case TYPES::VEDETTE:
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

	void Delete() noexcept
	{
		_currentState = quint8{};
	}

	explicit operator quint8() const noexcept
	{
		return _currentState;
	}

private:

	quint8 _currentState{ 0 };
};