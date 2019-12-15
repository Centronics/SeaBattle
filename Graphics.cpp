#include "stdafx.h"
#include "Packet.h"
#include "Graphics.h"

using namespace std;

static const QFont DRAW_FONT_MARK("Courier New", Graphics::ObjectWidth - 5);
static const QFont DRAW_FONT("Times", Graphics::ObjectWidth - 5, QFont::Bold);
static const QFont TEXT_FONT("Times", 13);
static const QFont BIG_FONT("Arial", 15, QFont::Bold);

void Graphics::Paint(QPainter& painter, const Ship::TYPES ship, const Ship::ROTATE rotate) const
{
	DrawShips(painter, ship, rotate);
}

void Graphics::DrawMoveQuad(QPainter& painter)
{
	if (ConnectionStatus == CONNECTIONSTATUS::DISCONNECTED)
		return;

	static const QPen Bp(Qt::black, BetweenObjects);
	static const QPen Rp(Qt::red, BetweenObjects);
	static const QBrush Rb(Qt::red, Qt::SolidPattern);
	static const QPen Gp(Qt::green, BetweenObjects);
	static const QBrush Gb(Qt::green, Qt::SolidPattern);
	static const QPen Rpen(Qt::red, 3);
	static const QPen Gpen(Qt::green, 3);

	if (ConnectionStatus == CONNECTIONSTATUS::SERVER || ConnectionStatus == CONNECTIONSTATUS::CLIENT)
	{
		painter.setPen(Rpen);
		painter.setFont(BIG_FONT);
		if (ConnectionStatus == CONNECTIONSTATUS::SERVER)
			painter.drawText(776, 301, "ŒÊË‰‡ÌËÂ ÔÓ‰ÍÎ˛˜ÂÌËˇ.");
		if (ConnectionStatus == CONNECTIONSTATUS::CLIENT)
			painter.drawText(826, 301, "œÓ‰ÍÎ˛˜ÂÌËÂ...");
		return;
	}

	if (IsRivalMove)
	{
		painter.setPen(Rpen);
		painter.setFont(TEXT_FONT);
		painter.drawText(843, 266, "’Ó‰ ÒÓÔÂÌËÍ‡.");
		painter.setPen(Rp);
		painter.setBrush(Rb);
	}
	else
	{
		painter.setPen(Gpen);
		painter.setFont(TEXT_FONT);
		painter.drawText(866, 266, "¬‡¯ ıÓ‰.");
		painter.setPen(Gp);
		painter.setBrush(Gb);
	}

	painter.drawEllipse(872, 281, 50, 50);
	painter.setPen(Bp);
	painter.setBrush(Qt::NoBrush);
	painter.drawRect(827, 246, 140, 100);
}

Graphics::SHIPADDITION Graphics::AddShip(const Ship::TYPES ship, const Ship::ROTATE rotate)
{
	if (ConnectionStatus != CONNECTIONSTATUS::DISCONNECTED)
		return SHIPADDITION::INCORRECTMODE;
	if (IsReadyToPlay(ship))
		return SHIPADDITION::MANY;
	const auto xs = GetMassiveCoords();
	if (!get<0>(xs))
		return SHIPADDITION::NOCOORD;
	return AddOrRemove(get<1>(xs), get<2>(xs), ship, rotate);
}

Graphics::SHIPADDITION Graphics::RemoveShip()
{
	const auto xd = GetShipCoords();
	if (!get<0>(xd))
		return SHIPADDITION::NOCOORD;
	if (ConnectionStatus != CONNECTIONSTATUS::DISCONNECTED)
		return SHIPADDITION::INCORRECTMODE;
	if (AddOrRemove(get<1>(xd), get<2>(xd), Ship::TYPES::EMPTY, get<3>(xd)) != SHIPADDITION::OK)
		throw exception(__func__);
	return SHIPADDITION::OK;
}

bool Graphics::RivalHit(const quint8 coord)
{
	Ship& ship = _screenObjects.at(coord);
	_lastHitRival = coord;
	if (ship.GetBeat(Ship::BEAT::ME))
		ship.SetBit(Ship::BIT::BOTH);
	else
		ship.SetBit(Ship::BIT::RIVAL);
	return ship.GetHolding(Ship::HOLDING::ME);
}

Graphics::HITSTATUS Graphics::MyHit(const quint8 coord)
{
	Ship& ship = _screenObjects.at(coord);
	if (ship.GetBeat(Ship::BEAT::ME))
		return HITSTATUS::BUSY;
	if (IsDenyNearBeat(coord))
		return HITSTATUS::NONEFFECTIVE;
	_lastHitMy = coord;
	if (ship.GetBeat(Ship::BEAT::RIVAL))
		ship.SetBit(Ship::BIT::BOTH);
	else
		ship.SetBit(Ship::BIT::ME);
	return ship.GetHolding(Ship::HOLDING::RIVAL) ? HITSTATUS::OK : HITSTATUS::FAIL;
}

bool Graphics::IsReadyToPlay(const Ship::TYPES ship) const
{
	if (ship == Ship::TYPES::EMPTY)
		return Ship::GetMaxShipCount(Ship::TYPES::CRUISER) == GetShipCount(Ship::TYPES::CRUISER) &&
		Ship::GetMaxShipCount(Ship::TYPES::ESMINEC) == GetShipCount(Ship::TYPES::ESMINEC) &&
		Ship::GetMaxShipCount(Ship::TYPES::LINKOR) == GetShipCount(Ship::TYPES::LINKOR) &&
		Ship::GetMaxShipCount(Ship::TYPES::VEDETTE) == GetShipCount(Ship::TYPES::VEDETTE);
	return Ship::GetMaxShipCount(ship) == GetShipCount(ship);
}

Graphics::BROKEN Graphics::GetBroken() const
{
	const auto isRivals = [this]
	{
		for (const Ship& s : _screenObjects)
			if (s.GetHolding(Ship::HOLDING::RIVAL))
				return true;
		return false;
	};

	const auto broken = [this](const Ship::HOLDING hld)
	{
		const Ship::BEAT b = (hld == Ship::HOLDING::ME) ? Ship::BEAT::RIVAL : Ship::BEAT::ME;
		for (const Ship& s : _screenObjects)
			if (s.GetHolding(hld) && !s.GetBeat(b))
				return false;
		return true;
	};

	if (!isRivals())
		return BROKEN::NOTHING;

	if (broken(Ship::HOLDING::ME))
		return BROKEN::ME;
	if (broken(Ship::HOLDING::RIVAL))
		return BROKEN::RIVAL;
	return BROKEN::NOTHING;
}

/*tuple<bool, int, int> Graphics::GetPhysicalCoords() // ”¡–¿“‹ –¿Ã ” œ–» ”ƒ¿–≈, —ƒ≈À¿“‹  –”∆Œ  œŒÃ≈Õ‹ÿ≈, —Œ√À¿—Œ¬¿“‹ ÷¬≈“¿  –”∆ Œ¬ » Ã≈Õﬂ » œ–Œ“»¬Õ» ¿, —ƒ≈À¿“‹ Œ“Œ¡–¿∆≈Õ»≈ œ–Œ“»¬Õ» Œ¬ œŒ—À≈ Ã¿“◊¿
{
	if (CursorX < MarginX || CursorX >= MaxCoordX || CursorY < MarginY || CursorY >= MaxCoordY)
		return make_tuple(false, -1, -1);
	int mx = CursorX - MarginX, my = CursorY - MarginY;
	mx = mx - (mx % ObjectWidth);
	my = my - (my % ObjectWidth);
	mx += MarginX;
	my += MarginY;
	return make_tuple(true, mx, my);
}*/

tuple<bool, int, int> Graphics::GetMassiveCoords()
{
	const bool conn = ConnectionStatus != CONNECTIONSTATUS::DISCONNECTED;
	const int marginX = conn ? BigMargin : MarginX, maxCoordX = conn ? BigMaxCoordX : MaxCoordX;
	if (CursorX < marginX || CursorX >= maxCoordX || CursorY < MarginY || CursorY >= MaxCoordY)
		return make_tuple(false, -1, -1);
	int mx = CursorX - marginX, my = CursorY - MarginY;
	mx = mx - (mx % ObjectWidth);
	my = my - (my % ObjectWidth);
	mx /= ObjectWidth;
	my /= ObjectWidth;
	return make_tuple(true, mx, my);
}

tuple<bool, int, int, Ship::ROTATE> Graphics::GetShipCoords() const
{
	const optional<quint8> mas = GetCoord();
	if (CursorX < MarginX || CursorX >= MaxCoordX || CursorY < MarginY || CursorY >= MaxCoordY || !mas)
		return make_tuple(false, -1, -1, Ship::ROTATE::NIL);
	const Ship& ship = _screenObjects[*mas];
	if (ship.GetShipType() == Ship::TYPES::EMPTY)
		return make_tuple(false, -1, -1, Ship::ROTATE::NIL);
	const quint8 mx = *mas % 10, my = *mas / 10;
	if (const Ship::ROTATE r = ship.GetRotate(); r != Ship::ROTATE::NIL)
		return make_tuple(true, mx, my, r);
	for (int x = 0; x < 10; ++x)
		for (int y = 0; y < 10; ++y)
		{
			switch (const Ship& s = _screenObjects[(y * 10) + x]; s.GetRotate())
			{
			case Ship::ROTATE::STARTRIGHT:
				if (mx >= x && mx < (x + s.GetFloors()) && my == y)
					return make_tuple(true, x, y, Ship::ROTATE::STARTRIGHT);
				continue;
			case Ship::ROTATE::STARTDOWN:
				if (my >= y && my < (y + s.GetFloors()) && mx == x)
					return make_tuple(true, x, y, Ship::ROTATE::STARTDOWN);
				[[fallthrough]];
			case Ship::ROTATE::NIL:
				continue;
			default:
				throw exception(__func__);
			}
		}
	return make_tuple(false, -1, -1, Ship::ROTATE::NIL);
}

/*const Ship* Graphics::IsKilled(const quint8 coord) const
{
	if (coord > 99)
		return nullptr;
	const Ship* const ps = &_screenObjects[coord];
	const int floors = ps->GetFloors();
	if (floors <= 0)
		return nullptr;
	switch (ps->GetRotate())
	{
	case Ship::ROTATE::STARTRIGHT:
		for (const Ship *s = ps, *ms = s + floors; s < ms; ++s)
			if (!s->GetBeat(Ship::BEAT::RIVAL))
				return nullptr;
		return ps;
	case Ship::ROTATE::STARTDOWN:
		for (const Ship *s = ps, *ms = s + (floors * 10); s < ms; s += 10)
			if (!s->GetBeat(Ship::BEAT::RIVAL))
				return nullptr;
		return ps;
	default:
		throw exception(__func__);
	}
}*/

Ship Graphics::IsRivalKilled(const quint8 coord, bool* coordMas) const
{
	if (coord > 99)
		return Ship();

	const Ship& s = _screenObjects[coord];
	if (!s.GetHolding(Ship::HOLDING::RIVAL) || (coordMas && coordMas[coord]))
		return Ship();
	bool retEmpty = !s.GetBeat(Ship::BEAT::ME);

	struct result
	{
		result() = default;
		result(const result&) = delete;
		result(result&&) = default;
		~result() = default;
		result& operator=(const result&) = delete;
		result& operator=(result&&) = default;

		int ShipSize = 0;
		Ship::ROTATE ShipRotate = Ship::ROTATE::NIL;

		bool operator> (const result& a) const
		{
			return ShipSize > a.ShipSize;
		}

		operator Ship::TYPES() const
		{
			return Ship::GetShipTypeBySize(ShipSize);
		}

		operator Ship::ROTATE() const
		{
			return ShipRotate;
		}
	};

	if (coordMas)
		coordMas[coord] = true;

	bool cMas1[100] = { false };
	bool cMas2[100] = { false };

	const auto rX = [this, coord, &cMas1, &s, coordMas, &retEmpty]
	{
		int shipSize = 1;
		//for (int k = coord + 1, t = coord + 3, km = t > 100 ? 100 : t; k < km; ++k)
		{
			int k = coord + 1;
			if (k >= 100)
				return s.GetBeat(Ship::BEAT::ME) ? result{ 1, Ship::ROTATE::STARTRIGHT } : result{ 0, Ship::ROTATE::NIL };
			const Ship* ship = &_screenObjects[k];
			while (ship->GetHolding(Ship::HOLDING::RIVAL) && (((k % 10) != 0) || k == 0))
			{
				//cMas1[k] = true;
				if (coordMas)
					coordMas[k++] = true;
				if (ship->GetBeat(Ship::BEAT::ME))
					shipSize++;
				else
					retEmpty = true;
				ship++;
				//continue;
			}
			//break;
		}
		return retEmpty ? result{ 0, Ship::ROTATE::NIL } : result{ shipSize, Ship::ROTATE::STARTRIGHT };
	};

	const auto rY = [this, coord, &cMas2, &s, coordMas, &retEmpty]
	{
		int shipSize = 1;
		//for (int k = coord + 10, t = coord + 30, km = t > 100 ? 100 : t; k < km; k += 10)
		{
			int k = coord + 10;
			if (k >= 100)
				return s.GetBeat(Ship::BEAT::ME) ? result{ 1, Ship::ROTATE::STARTRIGHT } : result{ 0, Ship::ROTATE::NIL };
			const Ship* ship = &_screenObjects[k];
			while (ship->GetHolding(Ship::HOLDING::RIVAL) && k < 100)
			{
				//cMas2[k] = true;
				if (coordMas)
					coordMas[k] = true;
				k += 10;
				if (ship->GetBeat(Ship::BEAT::ME))
					shipSize++;
				else
					retEmpty = true;
				ship += 10;
				//continue;
			}
			//break;
		}
		return retEmpty ? result{ 0, Ship::ROTATE::NIL } : result{ shipSize, Ship::ROTATE::STARTDOWN };
	};

	const auto commit = [&cMas1, &cMas2, coordMas](const bool rX)
	{
		if (!coordMas)
			return;
		/*bool* b = rX ? cMas1 : cMas2;
		for (quint8 k = 0; k < 100; ++k)
			if (*b++)
				coordMas[k] = true;*/
	};

	Ship res;
	result aX = rX(), aY = rY();

	if (retEmpty)
		return Ship();

	result a;
	if (aX > aY)
	{
		a = move(aX);
		commit(true);
	}
	else
	{
		a = move(aY);
		commit(false);
	}
	res.SetShipType(a);
	res.SetRotate(a);
	res.SetShipHolder(s.GetShipHolder());
	return res;
}

bool Graphics::IsDenyNearBeat(const quint8 coord/*, bool* coordMas, const quint8 kx*/) const
{
	const quint8 cX = coord % 10, cY = coord / 10;

	/*const auto inShipRange = [cX, cY, coordMas](const quint8 x, const quint8 y, const quint8 floors, const Ship::ROTATE rotate)
	{
		/*if(coordMas)
		{
			quint8 k = (y*10)+x;
			if(coordMas[k])
				return true;
		}*/
		/*switch (rotate)
		{
		case Ship::ROTATE::STARTRIGHT:
			return (cY == y) && (cX >= x) && (cX < (x + floors));
		case Ship::ROTATE::STARTDOWN:
			return (cX == x) && (cY >= y) && (cY < (y + floors));
		default:
			return false;
		}
	};*/

	const auto inRange = [](const quint8 curFX, const quint8 curMX, const quint8 floors)
	{
		const int sX = curFX > 0 ? curFX - 1 : 0;
		const int fX = curFX + floors;
		const int mX = fX < 10 ? fX : 9;
		return curMX >= sX && curMX <= mX;
	};

	//vector<RivalShip> ships;
	//ships.reserve(100);

	bool shipCoords[100] = { false };

	for (quint8 k = 0/*kx*/; k < 100; ++k)
	{
		//IsRivalKilled(k, coordMas);

		const quint8 x = k % 10, y = k / 10;
		const Ship s = IsRivalKilled(k, shipCoords);

		//if(s.GetRotate() != Ship::ROTATE::NIL)


		switch (s.GetRotate())
		{
		case Ship::ROTATE::STARTRIGHT:

			if ((/*!inShipRange(x, y, s.GetFloors(), Ship::ROTATE::STARTRIGHT) &&*/ !shipCoords[coord] && (inRange(x, cX, s.GetFloors()) && inRange(y, cY, 1))))
				return true;
			//coordMas[coord] = true;



		/*if (coordMas)
		{
			//if (!shipCoords[coord])
			//coordMas[coord] = true;
			continue;
		}
	if (!coordMas)
		return true;*/
		//ships.emplace_back(RivalShip{ x, y });
		//continue;
			break;
		case Ship::ROTATE::STARTDOWN:
			if (/*!inShipRange(x, y, s.GetFloors(), Ship::ROTATE::STARTDOWN)) &&*/ !shipCoords[coord] && (inRange(x, cX, 1) && inRange(y, cY, s.GetFloors())))
				return true;
			//coordMas[coord] = true;



		/*if (coordMas)
		{
			//if (!shipCoords[coord])
			//coordMas[coord] = true;
			continue;
		}
	if (!coordMas)
		return true;*/
		//ships.emplace_back(RivalShip{ x, y });
		//continue;
		}
		//if (s.GetRotate() == Ship::ROTATE::NIL)
			//continue;
	}

	/*if(coordMas)
	for (quint8 k1 = 0; k1 < 100; ++k1)
	{
		const quint8 x = k1 % 10, y = k1 / 10;
		//if (coordMas[k1])
			for(quint8 k2 = 0; k2 < ships.size(); ++k2)
			if(ships[k2].X==x&&ships[k2].Y==y)
				ships.erase(ships.begin()+k2);
	}*/
	//return ships;

	return false;
}

void Graphics::DrawShips(QPainter& painter, const Ship::TYPES ship, const Ship::ROTATE rotate) const
{
	const auto drawField = [&painter]
	{
		const auto drawText = [&painter](const int x, const int y, const QString& s, const QPen& pen, const QFont& f)
		{
			painter.setPen(pen);
			painter.setFont(f);
			painter.drawText(x, y, s);
		};

		const auto proc = [&painter](const int x, const int y, const int mx, const int my, const QPen& pen)
		{
			painter.setPen(pen);
			painter.setRenderHint(QPainter::Antialiasing);
			painter.drawLine(x, y, mx, my);
		};

		const auto drawMarkingX = [&drawText](const int x, const int y, const QPen& pen)
		{
			drawText(x + 4, y, "¿", pen, DRAW_FONT_MARK);
			drawText(x + 37, y, "¡", pen, DRAW_FONT_MARK);
			drawText(x + 69, y, "¬", pen, DRAW_FONT_MARK);
			drawText(x + 102, y, "√", pen, DRAW_FONT_MARK);
			drawText(x + 133, y, "ƒ", pen, DRAW_FONT_MARK);
			drawText(x + 165, y, "≈", pen, DRAW_FONT_MARK);
			drawText(x + 197, y, "∆", pen, DRAW_FONT_MARK);
			drawText(x + 228, y, "«", pen, DRAW_FONT_MARK);
			drawText(x + 261, y, "»", pen, DRAW_FONT_MARK);
			drawText(x + 292, y, " ", pen, DRAW_FONT_MARK);
		};

		const auto drawMarkingY = [&drawText](const int x, const int y, const QPen& pen)
		{
			drawText(x - 3, y + 16, "1", pen, DRAW_FONT_MARK);
			drawText(x - 3, y + 49, "2", pen, DRAW_FONT_MARK);
			drawText(x - 3, y + 80, "3", pen, DRAW_FONT_MARK);
			drawText(x - 3, y + 113, "4", pen, DRAW_FONT_MARK);
			drawText(x - 3, y + 144, "5", pen, DRAW_FONT_MARK);
			drawText(x - 3, y + 176, "6", pen, DRAW_FONT_MARK);
			drawText(x - 3, y + 208, "7", pen, DRAW_FONT_MARK);
			drawText(x - 3, y + 240, "8", pen, DRAW_FONT_MARK);
			drawText(x - 3, y + 272, "9", pen, DRAW_FONT_MARK);
			drawText(x - 24, y + 304, "10", pen, DRAW_FONT_MARK);
		};

		{
			const QPen blue(QColor(100, 129, 181), BetweenObjects);
			drawMarkingX(MarginX, 30, blue);
			drawMarkingY(MarginX - 25, 50, blue);

			for (int x = 0, yc = MarginY, xc = MarginX + (10 * ObjectWidth); x < 11; ++x)
			{
				proc(MarginX, yc, xc, yc, blue);
				yc += ObjectWidth;
			}

			for (int y = 0, xc = MarginX, yc = MarginY + (10 * ObjectWidth); y < 11; ++y)
			{
				proc(xc, MarginY, xc, yc, blue);
				xc += ObjectWidth;
			}
		}

		{
			const QPen red(ConnectionStatus == CONNECTIONSTATUS::DISCONNECTED ? QColor(216, 216, 216) : QColor(206, 96, 45), BetweenObjects);
			drawMarkingX(BigMargin, 30, red);
			drawMarkingY(BigMargin - 25, 50, red);

			for (int x = 0, yc = MarginY, xc = BigMargin + (10 * ObjectWidth); x < 11; ++x)
			{
				proc(BigMargin, yc, xc, yc, red);
				yc += ObjectWidth;
			}

			for (int y = 0, xc = BigMargin, yc = MarginY + (10 * ObjectWidth); y < 11; ++y)
			{
				proc(xc, MarginY, xc, yc, red);
				xc += ObjectWidth;
			}
		}
	};

	static const QPen G(Qt::gray, BetweenObjects);
	static const QBrush D(Qt::gray, Qt::Dense6Pattern);
	static constexpr int W = BetweenObjects * 2;

	int xMarkCoord = -1, yMarkCoord = -1, wMarkCoord = -1, hMarkCoord = -1;

	const auto grey = [&painter] { painter.setPen(G); painter.setBrush(D); };

	const auto marking = [&painter, &grey, &xMarkCoord, &yMarkCoord, &wMarkCoord, &hMarkCoord]
	{
		if (xMarkCoord < 0 || yMarkCoord < 0)
			return;
		if (wMarkCoord < 0 || hMarkCoord < 0)
			wMarkCoord = hMarkCoord = ObjectWidth;
		grey();
		//if (ConnectionStatus != CONNECTIONSTATUS::DISCONNECTED)
			//xMarkCoord += BigMaxCoordX - MaxCoordX;
		painter.drawRect(xMarkCoord - BetweenObjects, yMarkCoord - BetweenObjects, wMarkCoord + W, hMarkCoord + W);
	};

	int dWarnX = -1, dWarnY = -1;

	const auto drawWarning = [&dWarnX, &dWarnY]
	{
		dWarnX = CursorX;
		dWarnY = CursorY;
	};

	const auto isShipBeat = [this](const int x, const int y)
	{
		const Ship* s = &_screenObjects[(y * 10) + x];
		const Ship::ROTATE r = s->GetRotate();
		if (r != Ship::ROTATE::STARTRIGHT && r != Ship::ROTATE::STARTDOWN)
		{
			if (r == Ship::ROTATE::NIL)
				return false;
			throw exception("isShipBeat");
		}
		for (int k = 0, floors = s->GetFloors(); k < floors; ++k, s = (r == Ship::ROTATE::STARTDOWN) ? s + 10 : s + 1)
			if (s->GetBeat(Ship::BEAT::RIVAL))
				return true;
		return false;
	};

	const auto drawShipAndFrame = [&painter, ship, rotate, &xMarkCoord, &yMarkCoord, &wMarkCoord, &hMarkCoord, &grey, &drawWarning, this, &isShipBeat](const int px, const int y, const int mx, const int my, const int w, const int h, const Ship& s, const bool drawRival)
	{
		const int x = px + MarginX;
		const int frameX = px + (ConnectionStatus != CONNECTIONSTATUS::DISCONNECTED ? BigMargin : MarginX);
		const int xw = x + w, yh = y + h;
		const bool inFrame = CursorX >= frameX && CursorX < (frameX + ObjectWidth) && CursorY >= y && CursorY < (y + ObjectWidth);
		const auto drawShip = [x, y, w, h, &xMarkCoord, &yMarkCoord, &wMarkCoord, &hMarkCoord, &painter](const bool now) { if (now) painter.drawRect(x - BetweenObjects, y - BetweenObjects, w + W, h + W); else { xMarkCoord = x; yMarkCoord = y; wMarkCoord = w; hMarkCoord = h; } };
		const bool inShip = CursorX >= x && CursorX < xw && CursorY >= y && CursorY < yh;
		const auto drawMark = [px, y, &xMarkCoord, &yMarkCoord]
		{
			const int sx = px + (ConnectionStatus == CONNECTIONSTATUS::CONNECTED ? BigMargin : MarginX);
			xMarkCoord = sx, yMarkCoord = y;
		};

		if ((xw > /*(ConnectionStatus != CONNECTIONSTATUS::DISCONNECTED ? BigMaxCoordX :*/ MaxCoordX || yh > MaxCoordY) && inShip && inFrame)
		{
			drawMark();
			if (ship != Ship::TYPES::EMPTY && rotate != Ship::ROTATE::NIL)
				drawWarning();
			return;
		}

		if (s.GetRotate() != Ship::ROTATE::NIL)
		{
			const QColor color = s.GetColor();
			painter.setPen(QPen(color, BetweenObjects));
			painter.setBrush(QBrush(color, drawRival ? Qt::NoBrush : (isShipBeat(mx, my) ? Qt::NoBrush : Qt::Dense6Pattern)));
			//if (/*IsRivalMove || */ConnectionStatus == CONNECTIONSTATUS::DISCONNECTED)
			drawShip(true);
			if (/*!IsRivalMove*/ ConnectionStatus == CONNECTIONSTATUS::DISCONNECTED && inShip && inFrame)
				drawMark();
		}

		if (!IsRivalMove && inFrame)
		{
			if (!inShip || s.GetShipType() == Ship::TYPES::EMPTY)
			{
				grey();
				drawShip(false);
			}
			else
				drawMark();
		}

		if (ConnectionStatus == CONNECTIONSTATUS::CONNECTED && !IsRivalMove && inFrame)
			drawMark();

		if (ConnectionStatus == CONNECTIONSTATUS::DISCONNECTED && inFrame && IsBusy(mx, my, ship, rotate))
			drawWarning();
	};

	const auto drawBall = [&painter](const int x, const int y, const QColor& color)
	{// œ–Œ¬≈–»“‹ ‘À¿√» —Œ—“ŒﬂÕ»ﬂ œŒƒ Àﬁ◊≈Õ»ﬂ » Œ“Œ¡–¿∆≈Õ»ﬂ  ”–—Œ–¿ Õ¿ —ŒŒ“¬≈“—“¬”ﬁŸ≈Ã œŒÀ≈!!
		/*static constexpr int Wd = ObjectWidth / 2;
		static constexpr int Wc = (Wd / 2);
		painter.setPen(QPen(color, Wd));
		painter.drawEllipse(x + Wc + 3, y + Wc + 3, Wd - 6, Wd - 6);
		painter.drawPoint(x + Wd, y + Wd);*/

		static constexpr int Wd = (ObjectWidth / 8) - 1;// - 3;
		static constexpr int Wc = ObjectWidth / 2;
		painter.setPen(QPen(color, Wd));
		//painter.drawEllipse(x + Wc/* + 3*/, y + (Wc - 1), Wd/* - 6*/, Wd/* - 6*/);
		//painter.setPen(QPen(Qt::black, Wd));

		const QPointF pf(x + Wc + 0.5, y + Wc + 0.5);
		painter.drawEllipse(pf, Wd + 0.5/* - 6*/, Wd/* - 6*/ + 0.5);

		//painter.setPen(QPen(Qt::red, Wd));
		//const QPointF pf1(x + (Wc+0.5)-1, y + (Wc+0.5)-1);

		//painter.drawPoint(pf1);//x + (ObjectWidth/2)/* + 3*/, y + ObjectWidth + 1);
		const QPointF pf2(x + (Wc + 0.5), y + (Wc + 0.5));
		painter.drawPoint(pf2);//x + (ObjectWidth/2)/* + 3*/, y + ObjectWidth + 1);
		//painter.drawPoint(x + (ObjectWidth/2)/* + 3*/, y + (ObjectWidth /2));

		painter.setPen(QPen(QColor(240, 240, 240), 1));
		const QPointF pf3(x + (ObjectWidth / 2) - 1 + 0.5/* + 3*/, y + (ObjectWidth / 2) - 4 + (Wc / 2) + 0.5);
		painter.drawPoint(pf3);
	};

	const auto mBeat = [&painter, this, &drawBall](const int x, const int y, const int masNumber)
	{
		if (ConnectionStatus != CONNECTIONSTATUS::CONNECTED)
			return;

		const Ship& s = _screenObjects[masNumber];

		/*const auto drawMark = [&painter](const int x, const int y)
		{
			static const QColor HitColor = Qt::red; //QColor(255, 168, 153);
			painter.setPen(QPen(HitColor, BetweenObjects));
			painter.setBrush(Qt::NoBrush);
			painter.drawRect(x - BetweenObjects, y - BetweenObjects, ObjectWidth + W, ObjectWidth + W);
		};*/

		const auto drawX = [&painter](const int x, const int y, const QColor& color)
		{
			painter.setPen(QPen(color, 3));
			painter.setFont(DRAW_FONT);
			painter.drawText(x + 3, y + (ObjectWidth - 4), "X");
		};

		const bool isDeny = IsDenyNearBeat(masNumber);
		static const QColor WeakHitCol = QColor(0xff, 0x7e, 0x67);
		//static const QColor StrongCol = Qt::green; // QColor(0x50, 0x32, 0x8B);
		static const QColor WeakCol = Qt::darkGreen; //QColor(165, 225, 165); //QColor(0xC3, 0xBD, 0xD8); ÷¬≈“ —»À‹ÕŒ ¡‹®“ ¬ √À¿«¿!!!
		const Ship::BIT bit = s.GetBit();
		static constexpr int Margin = BigMargin - MarginX;
		if (const int dx = Margin + x; bit == Ship::BIT::ME || bit == Ship::BIT::BOTH)
		{
			if (const QColor c = _lastHitMy == masNumber ? Qt::red : (isDeny ? WeakCol : WeakHitCol); s.GetHolding(Ship::HOLDING::RIVAL))
				drawX(dx, y, c);
			else
				drawBall(dx, y, c);
			/*if ()
				drawMark(dx, y);*/
		}
		else
			//{
			if (isDeny)
				drawBall(dx, y, Qt::blue);

		if (const QColor c = _lastHitRival == masNumber ? Qt::red : WeakHitCol; bit == Ship::BIT::RIVAL || bit == Ship::BIT::BOTH)
		{
			if (s.GetHolding(Ship::HOLDING::ME))
				drawX(x, y, c);
			else
				drawBall(x, y, c);
		}
		//}
	};

	//const auto drawKilledShips = [&painter, this, &drawBall]
	//{
	//	if (ConnectionStatus != CONNECTIONSTATUS::CONNECTED)
	//		return;
	//	//bool coordMas[100] = { false };
	//	//for (quint8 k = 0; k < 100; ++k)
	//	for (int mx = 0, x = BigMargin; mx < 10; ++mx, x += ObjectWidth)
	//		for (int my = 0, y = MarginY; my < 10; ++my, y += ObjectWidth)
	//		{
	//			//Q_UNUSED(IsAllowNearBeat(k/*, coordMas, 0*/));//coordMas œ–Œ¬≈–ﬂ“‹ «ƒ≈—‹
	//			//for (const auto& f : t)
	//			//for (quint8 k = 0; k < 100; ++k)
	//			{
	//				//static constexpr int Wd = ObjectWidth / 2;
	//				//static constexpr int Wc = (Wd / 2);
	//				//static const QPen B(Qt::blue, Wd);
	//				if (/*const Ship& s = _screenObjects[k];*/ IsDenyNearBeat((my * 10) + mx)  /*!coordMas[k]*/) // || (s.GetHolding(Ship::HOLDING::RIVAL)/* && s.GetBeat(Ship::BEAT::ME)*/))
	//					//continue;
	//				//{
	//					//const int x = ((k % 10) * ObjectWidth) + BigMargin, y = ((k / 10) * ObjectWidth) + MarginY;

	//					drawBall(x, y, Qt::blue);
	//					//const int x = (f.X * ObjectWidth) + BigMargin, y = (f.Y * ObjectWidth) + MarginY;
	//					//painter.setPen(B);
	//					//painter.drawEllipse(x + Wc + 3, y + Wc + 3, Wd - 6, Wd - 6);
	//					//painter.drawPoint(x + Wd, y + Wd);
	//				//}
	//			}
	//		}
	//};

	const auto draw = [&drawShipAndFrame](const int x, const int y, const int mx, const int my, const int w, const int h, const Ship& s, const bool drawRival)
	{
		//	if (ConnectionStatus == CONNECTIONSTATUS::CONNECTED /*&& !IsRivalMove*/ && s.GetHolding(Ship::HOLDING::RIVAL) && s.GetBeat(Ship::BEAT::ME))
			//	drawShipAndFrame(x, y, mx, my, ObjectWidth, ObjectWidth, s);
		if (/*(IsRivalMove || ConnectionStatus == CONNECTIONSTATUS::DISCONNECTED) &&*/ s.GetHolding(Ship::HOLDING::ME))
			drawShipAndFrame(x, y, mx, my, w, h, s, drawRival);
	};

	//drawKilledShips();

	for (int mx = 0, x = MarginX; mx < 10; ++mx, x += ObjectWidth)
		for (int my = 0, y = MarginY; my < 10; ++my, y += ObjectWidth)
			mBeat(x, y, (my * 10) + mx);

	drawField();

	for (int mx = 0, x = 0;/*MarginX*/ mx < 10; ++mx, x += ObjectWidth)
		for (int my = 0, y = MarginY; my < 10; ++my, y += ObjectWidth)
		{
			const Ship& s = _screenObjects[(my * 10) + mx];
			const Ship::ROTATE r = /*(!IsRivalMove && ConnectionStatus != CONNECTIONSTATUS::DISCONNECTED) ? Ship::ROTATE::NIL : */s.GetRotate();
			switch (const int floors = s.GetFloors() * ObjectWidth; r)
			{
			case Ship::ROTATE::STARTRIGHT:
				draw(x, y, mx, my, floors, ObjectWidth, s, false);
				continue;
			case Ship::ROTATE::STARTDOWN:
				draw(x, y, mx, my, ObjectWidth, floors, s, false);
				continue;
			case Ship::ROTATE::NIL:
				switch (const int fls = Ship::GetFloors(ship) * ObjectWidth; rotate)
				{
				case Ship::ROTATE::STARTRIGHT:
					drawShipAndFrame(x, y, mx, my, fls, ObjectWidth, s, false);
					continue;
				case Ship::ROTATE::STARTDOWN:
					drawShipAndFrame(x, y, mx, my, ObjectWidth, fls, s, false);
					continue;
				case Ship::ROTATE::NIL:
					drawShipAndFrame(x, y, mx, my, ObjectWidth, ObjectWidth, s, false);
					continue;
				default:
					throw exception(__func__);
				}
			default:
				throw exception(__func__);
			}
		}

	if (ConnectionStatus == CONNECTIONSTATUS::CONNECTED)
	{
		bool shipCoords[100] = { false };
		static constexpr int Margin = BigMargin - MarginX;

		for (int mx = 0, x = Margin; mx < 10; ++mx, x += ObjectWidth)
			for (int my = 0, y = MarginY; my < 10; ++my, y += ObjectWidth)
			{
				const Ship s = IsRivalKilled((my * 10) + mx, shipCoords);
				switch (const int floors = s.GetFloors() * ObjectWidth; s.GetRotate())
				{
				case Ship::ROTATE::STARTRIGHT:
					drawShipAndFrame(x, y, mx, my, floors, ObjectWidth, s, true);
					continue;
				case Ship::ROTATE::STARTDOWN:
					drawShipAndFrame(x, y, mx, my, ObjectWidth, floors, s, true);
				case Ship::ROTATE::NIL:
					continue;
				default:
					throw exception(__func__);
				}
			}
	}

	marking();

	if (ConnectionStatus == CONNECTIONSTATUS::CONNECTED)
	{
		/*const auto drawMark = [&painter](const int x, const int y)
		{
			static const QColor HitColor = QColor(255, 168, 153);
			painter.setPen(QPen(HitColor, BetweenObjects));
			painter.setBrush(Qt::NoBrush);
			painter.drawRect(x - BetweenObjects, y - BetweenObjects, ObjectWidth + W, ObjectWidth + W);
		};

		if (_lastHitMy < 100)
		{
			const int x = ((_lastHitMy % 10) * ObjectWidth) + BigMargin, y = ((_lastHitMy / 10) * ObjectWidth) + MarginY;
			drawMark(x, y);
		}
		if (_lastHitRival < 100)
		{
			const int x = ((_lastHitRival % 10) * ObjectWidth) + MarginX, y = ((_lastHitRival / 10) * ObjectWidth) + MarginY;
			drawBall(x, y, Qt::darkYellow);
		}*/
	}
	else
	{
		_lastHitMy = 255;
		_lastHitRival = 255;
	}

	if (dWarnX < 0 || dWarnY < 0)
		return;
	painter.setPen(QPen(Qt::red, 3));
	painter.setFont(DRAW_FONT);
	painter.drawText(dWarnX - 12, dWarnY + 20, "!");
}

bool Graphics::IsFree(const int sx, const int sy) const
{
	if (sx < 0 || sx > 9 || sy < 0 || sy > 9)
		throw exception(__func__);

	const auto coord = [this](const int x, const int y) -> bool
	{
		if (x < 0 || x > 9 || y < 0 || y > 9)
			return true;
		return !_screenObjects[(y * 10) + x].GetHolding(Ship::HOLDING::ME);
	};

	if (!coord(sx - 1, sy))
		return false;
	if (!coord(sx + 1, sy))
		return false;
	if (!coord(sx, sy - 1))
		return false;
	if (!coord(sx, sy + 1))
		return false;
	if (!coord(sx, sy))
		return false;
	if (!coord(sx - 1, sy - 1))
		return false;
	if (!coord(sx - 1, sy + 1))
		return false;
	if (!coord(sx + 1, sy - 1))
		return false;
	if (!coord(sx + 1, sy + 1))
		return false;

	return true;
}

bool Graphics::IsBusy(const int startX, const int startY, const Ship::TYPES ship, const Ship::ROTATE rotate) const
{
	if (startX < 0 || startX > 9 || startY < 0 || startY > 9 || ship == Ship::TYPES::EMPTY || rotate == Ship::ROTATE::NIL)
		return false;
	switch (rotate)
	{
	case Ship::ROTATE::STARTDOWN:
	{
		const int max = startY + Ship::GetFloors(ship);
		if (max > 10)
			return false;
		for (int k = startY; k < max; ++k)
			if (!IsFree(startX, k))
				return true;
		return false;
	}
	case Ship::ROTATE::STARTRIGHT:
	{
		const int max = startX + Ship::GetFloors(ship);
		if (max > 10)
			return false;
		for (int k = startX; k < max; ++k)
			if (!IsFree(k, startY))
				return true;
		return false;
	}
	case Ship::ROTATE::NIL:
	default:
		throw exception(__func__);
	}
}

Graphics::SHIPADDITION Graphics::AddOrRemove(const int startX, const int startY, const Ship::TYPES ship, const Ship::ROTATE rotate)
{
	if (ConnectionStatus != CONNECTIONSTATUS::DISCONNECTED)
		return SHIPADDITION::INCORRECTMODE;

	if (_screenObjects.size() != 100 || startX < 0 || startX > 9 || startY < 0 || startY > 9)
		throw exception(__func__);

	int i = (startY * 10) + startX;

	const auto test = [this, ship, rotate, startX, startY, i](const int coord, int& floors, SHIPADDITION& sa)
	{
		floors = Ship::GetFloors(ship);
		if ((coord + floors) > 10)
		{
			sa = SHIPADDITION::NOCOORD;
			return false;
		}
		if (ship != Ship::TYPES::EMPTY)
		{
			const bool b = IsBusy(startX, startY, ship, rotate);
			sa = b ? SHIPADDITION::NOTFREE : SHIPADDITION::OK;
			return !b;
		}
		sa = SHIPADDITION::OK;
		if (const int c = _screenObjects[i].GetFloors(); c != 0)
		{
			floors = c;
			return true;
		}
		return false;
	};

	const auto item = [this, ship, &i, rotate](const bool start)
	{
		if (Ship& obj = _screenObjects[i]; ship != Ship::TYPES::EMPTY)
		{
			obj.SetRotate(start ? rotate : Ship::ROTATE::NIL);
			obj.SetBit(Ship::BIT::NIL);
			obj.SetShipHolder(Ship::HOLDER::ME);
			obj.SetShipType(ship);
		}
		else
			obj.Delete();
	};

	switch (int floors = 0; rotate)
	{
	case Ship::ROTATE::STARTRIGHT:
		if (SHIPADDITION sa; !test(startX, floors, sa))
			return sa;
		for (int k = 0; k < floors; ++k, ++i)
			item(!k);
		return SHIPADDITION::OK;
	case Ship::ROTATE::STARTDOWN:
		if (SHIPADDITION sa; !test(startY, floors, sa))
			return sa;
		for (int k = 0; k < floors; ++k, i += 10u)
			item(!k);
		return SHIPADDITION::OK;
	default:
		throw exception(__func__);
	}
}

int Graphics::GetShipCount(const Ship::TYPES ship) const
{
	int result = 0;
	for (const auto& obj : _screenObjects)
		if (obj.GetShipType() == ship && obj.GetRotate() != Ship::ROTATE::NIL)
			result++;
	return result;
}

void Graphics::ClearBitShips()
{
	for (auto& obj : _screenObjects)
		obj.SetBit(Ship::BIT::NIL);
}

optional<quint8> Graphics::GetCoord() const
{
	const auto r = GetMassiveCoords();
	if (!get<0>(r))
		return nullopt;
	return (get<2>(r) * 10) + get<1>(r);
}

vector<Ship>& Graphics::GetData()
{
	return _screenObjects;
}
