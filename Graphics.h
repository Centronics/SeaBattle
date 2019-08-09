#pragma once
#include "Ship.h"

class Graphics : public QObject
{
	Q_OBJECT

public:

	enum class SHIPADDITION
	{
		OK,
		MANY,
		NOCOORD,
		NOSHIP,
		NOTFREE,
		INCORRECTMODE
	};

protected:

	std::vector<Ship> _screenObjects{ 100 };

	[[nodiscard]] bool IsFree(int sx, int sy) const;
	void DrawShipRect(QPainter& painter, Ship::SHIPTYPES ship, Ship::ROTATE rotate) const;
	[[nodiscard]] SHIPADDITION AddOrRemove(int startX, int startY, Ship::SHIPTYPES ship, Ship::ROTATE rotate);
	[[nodiscard]] bool IsKilled(quint8 coord, Ship::BIT bit) const;
	static void DrawField(QPainter& painter);
	[[nodiscard]] static std::tuple<bool, int, int> GetPhysicalCoords();
	[[nodiscard]] static std::tuple<bool, int, int> GetMassiveCoords();
	static void SetMoveQuad(QPainter& painter);

public:

	static constexpr int Margin = 10, BetweenObjects = 5, ObjectWidth = 32, MaxCoord = (ObjectWidth * 10) + Margin;

	explicit Graphics(QObject* parent) : QObject(parent) { }
	~Graphics() = default;
	Graphics(const Graphics&) = delete;
	Graphics(Graphics&&) = delete;
	Graphics& operator=(const Graphics&) = delete;
	Graphics& operator=(Graphics&&) = delete;

	inline static bool Clicked = false, IsRivalMove = false, ShipAddition = true;
	inline static int CursorX = -1, CursorY = 0;

	void Paint(QPainter& painter, Ship::SHIPTYPES ship = Ship::SHIPTYPES::EMPTY, Ship::ROTATE rotate = Ship::ROTATE::NIL) const;
	void ClearRivalState();
	void ClearField();
	void RivalHit(quint8 coord);
	void MyHit(quint8 coord);

	[[nodiscard]] SHIPADDITION RemoveShip();
	[[nodiscard]] SHIPADDITION AddShip(Ship::SHIPTYPES ship, Ship::ROTATE rotate);
	[[nodiscard]] bool IsReadyToPlay(Ship::SHIPTYPES ship = Ship::SHIPTYPES::EMPTY) const;
	[[nodiscard]] bool IsRivalBroken() const;
	[[nodiscard]] bool IsIamBroken() const;
	[[nodiscard]] int GetShipCount(Ship::SHIPTYPES ship) const;
	[[nodiscard]] std::optional<quint8> GetCoord() const;
	[[nodiscard]] std::vector<Ship>& GetData();
	[[nodiscard]] bool ReadRivals(const Packet& packet);
};