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

protected:

	std::vector<Ship> _screenObjects{ 100 };

	[[nodiscard]] bool IsFree(int sx, int sy) const;
	[[nodiscard]] bool IsBusy(int startX, int startY, Ship::TYPES ship, Ship::ROTATE rotate) const;
	void DrawShipRect(QPainter& painter, Ship::TYPES ship, Ship::ROTATE rotate) const;
	[[nodiscard]] SHIPADDITION AddOrRemove(int startX, int startY, Ship::TYPES ship, Ship::ROTATE rotate);
	static void DrawField(QPainter& painter);
	[[nodiscard]] static std::tuple<bool, int, int> GetPhysicalCoords();
	[[nodiscard]] static std::tuple<bool, int, int> GetMassiveCoords();
	[[nodiscard]] std::tuple<bool, int, int, Ship::ROTATE> GetShipCoords() const;
	static void DrawWarning(QPainter& painter);

public:

	static constexpr int Margin = 10, BetweenObjects = 5, ObjectWidth = 32, MaxCoord = (ObjectWidth * 10) + Margin;

	Graphics() = default;
	~Graphics() = default;
	Graphics(const Graphics&) = delete;
	Graphics(Graphics&&) = delete;
	Graphics& operator=(const Graphics&) = delete;
	Graphics& operator=(Graphics&&) = delete;

	inline static bool IsClicked = false, IsRivalMove = false, IsShipAddition = true, IsConnected = false;
	inline static int CursorX = -1, CursorY = 0;

	void Paint(QPainter& painter, Ship::TYPES ship = Ship::TYPES::EMPTY, Ship::ROTATE rotate = Ship::ROTATE::NIL) const;
	void ClearRivalState();
	void ClearField();
	bool RivalHit(quint8 coord);
	bool MyHit(quint8 coord);

	static void DrawMoveQuad(QPainter& painter);

	[[nodiscard]] SHIPADDITION RemoveShip();
	[[nodiscard]] SHIPADDITION AddShip(Ship::TYPES ship, Ship::ROTATE rotate);
	[[nodiscard]] bool IsReadyToPlay(Ship::TYPES ship = Ship::TYPES::EMPTY) const;
	[[nodiscard]] BROKEN GetBroken() const;
	[[nodiscard]] int GetShipCount(Ship::TYPES ship) const;
	[[nodiscard]] std::optional<quint8> GetCoord() const;
	[[nodiscard]] std::vector<Ship>& GetData();
};