#pragma once
#include "Ship.h"

class Graphics
{
	friend class Server;

protected:

	std::vector<Ship> _screenObjects{ 100 };

	[[nodiscard]] bool IsFree(int sx, int sy) const;
	void DrawFrame(QPainter& painter, Ship::SHIPS ship, Ship::STATE state) const;
	void DrawShipState(QPainter& painter) const;
	[[nodiscard]] std::tuple<bool, int, int, int, int> GetShipRect(bool realObject, Ship::SHIPS ship, Ship::STATE state) const;
	[[nodiscard]] bool AddOrRemove(int startX, int startY, Ship::SHIPS ship = Ship::SHIPS::EMPTY, Ship::STATE state = Ship::STATE::NIL);
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

	void Paint(QPainter& painter, Ship::SHIPS ship, Ship::STATE state) const;
	void ClearRivalState();
	void ClearField();
	[[nodiscard]] bool AddShip(Ship::SHIPS ship, Ship::STATE state);
	void RemoveShip();
	[[nodiscard]] bool IsReady(Ship::SHIPS ship = Ship::SHIPS::EMPTY) const;
	[[nodiscard]] int GetShipCount(Ship::SHIPS ship) const;
	[[nodiscard]] std::optional<quint8> GetCoord() const;
	[[nodiscard]] const std::vector<Ship>& GetData() const;
};