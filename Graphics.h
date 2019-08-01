#pragma once
#include "Ship.h"

class Graphics
{
	friend class Server;

protected:

	std::vector<Ship> _screenObjects{ 100 };

	[[nodiscard]] bool IsFree(int sx, int sy) const;
	void DrawShipRect(QPainter& painter, std::optional<const Ship::SHIPS> ship, Ship::ROTATE rotate) const;
	[[nodiscard]] bool AddOrRemove(int startX, int startY, std::optional<Ship::SHIPS> ship, Ship::ROTATE rotate = Ship::ROTATE::NIL);
	static void DrawField(QPainter& painter);
	[[nodiscard]] static std::tuple<bool, int, int> GetPhysicalCoords();
	[[nodiscard]] static std::tuple<bool, int, int> GetMassiveCoords();

public:

	static constexpr int Margin = 10, BetweenObjects = 5, ObjectWidth = 32, MaxCoord = (ObjectWidth * 10) + Margin;

	Graphics() = default;
	~Graphics() = default;
	Graphics(const Graphics&) = delete;
	Graphics(Graphics&&) = delete;
	Graphics& operator=(const Graphics&) = delete;
	Graphics& operator=(Graphics&&) = delete;

	inline static bool Clicked = false, ShipAddition = true, IsRivalMove = false;
	inline static int CursorX = -1, CursorY = 0;

	void Paint(QPainter& painter, std::optional<Ship::SHIPS> ship = std::nullopt, Ship::ROTATE rotate = Ship::ROTATE::NIL) const;
	void ClearRivalState();
	void ClearField();
	[[nodiscard]] bool AddShip(Ship::SHIPS ship, Ship::ROTATE rotate);
	void RemoveShip();
	[[nodiscard]] bool IsReady(std::optional<Ship::SHIPS> ship = std::nullopt) const;
	[[nodiscard]] int GetShipCount(Ship::SHIPS ship) const;
	[[nodiscard]] std::optional<quint8> GetCoord() const;
	[[nodiscard]] std::vector<Ship>& GetData();
};