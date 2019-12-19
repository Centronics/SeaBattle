#pragma once
#include "Ship.h"

class Graphics
{
public:

	enum class SHIPADDITION
	{
		OK,
		MANY,
		NOCOORD,
		NOTFREE,
		INCORRECTMODE
	};

	enum class BROKEN
	{
		NOTHING,
		ME,
		RIVAL
	};

	enum class CONNECTIONSTATUS
	{
		DISCONNECTED,
		SERVER,
		CLIENT,
		CONNECTED
	};

	enum class HITSTATUS
	{
		OK,
		FAIL,
		BUSY,
		NONEFFECTIVE
	};

protected:

	[[nodiscard]] bool IsFree(int sx, int sy) const;
	[[nodiscard]] bool IsBusy(int startX, int startY, Ship::TYPES ship, Ship::ROTATE rotate) const;
	[[nodiscard]] Ship GetRivalShip(quint8 coord, bool* coordMas, bool isKilled) const;
	[[nodiscard]] bool IsDenyNearBeat(quint8 coord) const;
	void DrawShips(QPainter& painter, Ship::TYPES ship, Ship::ROTATE rotate) const;
	[[nodiscard]] SHIPADDITION AddOrRemove(int startX, int startY, Ship::TYPES ship, Ship::ROTATE rotate);
	[[nodiscard]] static std::tuple<bool, int, int> GetMassiveCoords();
	[[nodiscard]] std::tuple<bool, int, int, Ship::ROTATE> GetShipCoords() const;

public:

	static constexpr int MarginX = 55, MarginY = 40, BigMargin = 432, BetweenObjects = 5, ObjectWidth = 32, MaxCoordX = (ObjectWidth * 10) + MarginX, BigMaxCoordX = (ObjectWidth * 10) + BigMargin, MaxCoordY = (ObjectWidth * 10) + MarginY;

	Graphics() = default;
	~Graphics() = default;
	Graphics(const Graphics&) = delete;
	Graphics(Graphics&&) = delete;
	Graphics& operator=(const Graphics&) = delete;
	Graphics& operator=(Graphics&&) = delete;

	inline static bool IsRivalMove = false;
	inline static int CursorX = -1, CursorY = 0;
	inline static CONNECTIONSTATUS ConnectionStatus = CONNECTIONSTATUS::DISCONNECTED;

	void Paint(QPainter& painter, Ship::TYPES ship, Ship::ROTATE rotate) const;
	bool RivalHit(quint8 coord);
	HITSTATUS MyHit(quint8 coord);

	static void DrawMoveQuad(QPainter& painter);

	[[nodiscard]] SHIPADDITION RemoveShip();
	[[nodiscard]] SHIPADDITION AddShip(Ship::TYPES ship, Ship::ROTATE rotate);
	[[nodiscard]] bool IsReadyToPlay(Ship::TYPES ship = Ship::TYPES::EMPTY) const;
	[[nodiscard]] BROKEN GetBroken() const;
	[[nodiscard]] int GetShipCount(Ship::TYPES ship) const;
	[[nodiscard]] std::optional<quint8> GetCoord() const;
	[[nodiscard]] std::vector<Ship>& GetData();
	void ClearBitShips();
	void DrawRivals();

private:

	std::vector<Ship> _screenObjects{ 100 };
	mutable quint8 _lastHitMy = 255, _lastHitRival = 255;
	bool _isDrawRivals = false;
};