#pragma once
#include "Ship.h"

class Graphics : public QObject
{
	Q_OBJECT

	bool _shipsAddition = true;

protected:

	std::vector<Ship> _screenObjects{ 100 };

	[[nodiscard]] bool IsFree(int sx, int sy) const;
	void DrawShipRect(QPainter& painter, Ship::SHIPTYPES ship, Ship::ROTATE rotate) const;
	[[nodiscard]] bool AddOrRemove(int startX, int startY, Ship::SHIPTYPES ship, Ship::ROTATE rotate);
	static void DrawField(QPainter& painter);
	[[nodiscard]] static std::tuple<bool, int, int> GetPhysicalCoords();
	[[nodiscard]] static std::tuple<bool, int, int> GetMassiveCoords();
	[[nodiscard]] bool IsKilled(unsigned int k, Ship::BIT bit) const;

public:

	static constexpr int Margin = 10, BetweenObjects = 5, ObjectWidth = 32, MaxCoord = (ObjectWidth * 10) + Margin;

	explicit Graphics(QObject* parent) : QObject(parent) { }
	~Graphics() = default;
	Graphics(const Graphics&) = delete;
	Graphics(Graphics&&) = delete;
	Graphics& operator=(const Graphics&) = delete;
	Graphics& operator=(Graphics&&) = delete;

	inline static bool Clicked = false, IsRivalMove = false;
	inline static int CursorX = -1, CursorY = 0;

	void Paint(QPainter& painter, Ship::SHIPTYPES ship = Ship::SHIPTYPES::EMPTY, Ship::ROTATE rotate = Ship::ROTATE::NIL) const;
	void ClearRivalState();
	void ClearField();
	[[nodiscard]] bool AddShip(Ship::SHIPTYPES ship, Ship::ROTATE rotate);
	void RemoveShip();
	[[nodiscard]] bool IsReadyToPlay(Ship::SHIPTYPES ship = Ship::SHIPTYPES::EMPTY) const;
	[[nodiscard]] bool IsRivalBroken() const;
	[[nodiscard]] bool IsIamBroken() const;
	[[nodiscard]] int GetShipCount(Ship::SHIPTYPES ship) const;
	[[nodiscard]] std::optional<quint8> GetCoord() const;
	[[nodiscard]] std::vector<Ship>& GetData();
	[[nodiscard]] bool ReadRivals(const Packet& packet);
	[[nodiscard]] bool IsShipsAddition() const { return _shipsAddition; }

public slots:

	void SlotShipsAdded(bool added);
};