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
			painter.drawText(776, 301, "Ожидание подключения.");
		if (ConnectionStatus == CONNECTIONSTATUS::CLIENT)
			painter.drawText(826, 301, "Подключение...");
		return;
	}

	if (IsRivalMove)
	{
		painter.setPen(Rpen);
		painter.setFont(TEXT_FONT);
		painter.drawText(843, 266, "Ход соперника.");
		painter.setPen(Rp);
		painter.setBrush(Rb);
	}
	else
	{
		painter.setPen(Gpen);
		painter.setFont(TEXT_FONT);
		painter.drawText(866, 266, "Ваш ход.");
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

Ship Graphics::GetRivalShip(const quint8 coord, bool* coordMas, const bool isKilled) const
{
	if (coord > 99)
		return Ship();

	const Ship& s = _screenObjects[coord];
	if (!s.GetHolding(Ship::HOLDING::RIVAL) || (coordMas && coordMas[coord]))
		return Ship();
	bool retEmpty = !s.GetBeat(Ship::BEAT::ME) && isKilled;

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

	const auto rX = [this, coord, &cMas1, &s, coordMas, &retEmpty, isKilled]
	{
		int shipSize = 1;
		int k = coord + 1;
		if (k >= 100)
			return !isKilled || s.GetBeat(Ship::BEAT::ME) ? result{ 1, Ship::ROTATE::STARTRIGHT } : result{ 0, Ship::ROTATE::NIL };
		const Ship* ship = &_screenObjects[k];
		while (ship->GetHolding(Ship::HOLDING::RIVAL) && (((k % 10) != 0) || k == 0))
		{
			if (coordMas)
				coordMas[k++] = true;
			if (ship->GetBeat(Ship::BEAT::ME) || !isKilled)
				shipSize++;
			else
				retEmpty = true;
			ship++;
		}
		return retEmpty ? result{ 0, Ship::ROTATE::NIL } : result{ shipSize, Ship::ROTATE::STARTRIGHT };
	};

	const auto rY = [this, coord, &cMas2, &s, coordMas, &retEmpty, isKilled]
	{
		int shipSize = 1;
		{
			int k = coord + 10;
			if (k >= 100)
				return !isKilled || s.GetBeat(Ship::BEAT::ME) ? result{ 1, Ship::ROTATE::STARTRIGHT } : result{ 0, Ship::ROTATE::NIL };
			const Ship* ship = &_screenObjects[k];
			while (ship->GetHolding(Ship::HOLDING::RIVAL) && k < 100)
			{
				if (coordMas)
					coordMas[k] = true;
				k += 10;
				if (ship->GetBeat(Ship::BEAT::ME) || !isKilled)
					shipSize++;
				else
					retEmpty = true;
				ship += 10;
			}
		}
		return retEmpty ? result{ 0, Ship::ROTATE::NIL } : result{ shipSize, Ship::ROTATE::STARTDOWN };
	};

	Ship res;
	result aX = rX(), aY = rY();

	if (retEmpty)
		return Ship();

	result a;
	if (aX > aY)
		a = move(aX);
	else
		a = move(aY);
	res.SetShipType(a);
	res.SetRotate(a);
	res.SetShipHolder(s.GetShipHolder());
	return res;
}

bool Graphics::IsDenyNearBeat(const quint8 coord) const
{
	const quint8 cX = coord % 10, cY = coord / 10;

	const auto inRange = [](const quint8 curFX, const quint8 curMX, const quint8 floors)
	{
		const int sX = curFX > 0 ? curFX - 1 : 0;
		const int fX = curFX + floors;
		const int mX = fX < 10 ? fX : 9;
		return curMX >= sX && curMX <= mX;
	};

	bool shipCoords[100] = { false };

	for (quint8 k = 0/*kx*/; k < 100; ++k)
	{
		const quint8 x = k % 10, y = k / 10;
		const Ship s = GetRivalShip(k, shipCoords, true);
		switch (s.GetRotate())
		{
		case Ship::ROTATE::STARTRIGHT:

			if (!shipCoords[coord] && (inRange(x, cX, s.GetFloors()) && inRange(y, cY, 1)))
				return true;
			break;
		case Ship::ROTATE::STARTDOWN:
			if (!shipCoords[coord] && (inRange(x, cX, 1) && inRange(y, cY, s.GetFloors())))
				return true;
		}
	}
	return false;
}

void Graphics::DrawShips(QPainter& painter, const Ship::TYPES ship, const Ship::ROTATE rotate) const
{
	const bool dr = _isDrawRivals;

	const auto drawField = [&painter, dr]
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
			drawText(x + 4, y, "А", pen, DRAW_FONT_MARK);
			drawText(x + 37, y, "Б", pen, DRAW_FONT_MARK);
			drawText(x + 69, y, "В", pen, DRAW_FONT_MARK);
			drawText(x + 102, y, "Г", pen, DRAW_FONT_MARK);
			drawText(x + 133, y, "Д", pen, DRAW_FONT_MARK);
			drawText(x + 165, y, "Е", pen, DRAW_FONT_MARK);
			drawText(x + 197, y, "Ж", pen, DRAW_FONT_MARK);
			drawText(x + 228, y, "З", pen, DRAW_FONT_MARK);
			drawText(x + 261, y, "И", pen, DRAW_FONT_MARK);
			drawText(x + 292, y, "К", pen, DRAW_FONT_MARK);
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

		const QPen red(ConnectionStatus == CONNECTIONSTATUS::CONNECTED || dr ? QColor(206, 96, 45) : QColor(216, 216, 216), BetweenObjects);
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
	};

	static const QPen G(Qt::gray, BetweenObjects);
	static const QBrush D(Qt::gray, Qt::Dense6Pattern);
	static constexpr int W = BetweenObjects * 2;

	int xMarkCoord = -1, yMarkCoord = -1, wMarkCoord = -1, hMarkCoord = -1;

	const auto grey = [&painter] { painter.setPen(G); painter.setBrush(D); };

	const auto marking = [&painter, &grey, &xMarkCoord, &yMarkCoord, &wMarkCoord, &hMarkCoord]
	{
		if (ConnectionStatus != CONNECTIONSTATUS::CONNECTED && ConnectionStatus != CONNECTIONSTATUS::DISCONNECTED)
			return;
		if (xMarkCoord < 0 || yMarkCoord < 0)
			return;
		if (wMarkCoord < 0 || hMarkCoord < 0 || ConnectionStatus == CONNECTIONSTATUS::CONNECTED)
			wMarkCoord = hMarkCoord = ObjectWidth;
		grey();
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
		const auto drawMark = [px, y, &xMarkCoord, &yMarkCoord] { xMarkCoord = px + (ConnectionStatus == CONNECTIONSTATUS::CONNECTED ? BigMargin : MarginX); yMarkCoord = y; };

		const auto drawS = [&painter, &s, drawRival, &isShipBeat, &drawShip, &drawMark, inShip, inFrame, mx, my](const bool dMrk)
		{
			if (s.GetRotate() != Ship::ROTATE::NIL)
			{
				const QColor color = s.GetColor();
				painter.setPen(QPen(color, BetweenObjects));
				painter.setBrush(QBrush(color, drawRival ? Qt::NoBrush : (isShipBeat(mx, my) ? Qt::NoBrush : Qt::Dense6Pattern)));
				drawShip(true);
				if (dMrk && ConnectionStatus == CONNECTIONSTATUS::DISCONNECTED && inShip && inFrame)
					drawMark();
			}
		};

		if ((xw > MaxCoordX || yh > MaxCoordY) && inShip && inFrame)
		{
			if (px > 0 && ConnectionStatus != CONNECTIONSTATUS::CONNECTED && s.GetRotate() != Ship::ROTATE::NIL)
			{
				drawS(false);
				return;
			}
			drawMark();
			if (ship != Ship::TYPES::EMPTY && rotate != Ship::ROTATE::NIL)
				drawWarning();
			return;
		}

		drawS(true);

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

	const auto mBeat = [&painter, this, dr](const int x, const int y, const int masNumber)
	{
		/*const auto getPixel = [](const int x, const int y)
		{
			
		};*/

		const auto drawBall = [&painter/*, &getPixel*/](const int x, const int y, const QColor& color)
		{
			static constexpr int Wd = (ObjectWidth / 8) - 1;
			static constexpr int Wc = ObjectWidth / 2;
			painter.setPen(QPen(color, Wd));
			const QPointF pf(x + Wc + 0.5, y + Wc + 0.5);
			painter.drawEllipse(pf, Wd + 0.5, Wd + 0.5);
			const QPointF pf2(x + (Wc + 0.5), y + (Wc + 0.5));
			painter.drawPoint(pf2);

			extern QColor GlobalColor;
			
			QColor c = GlobalColor; //getPixel(x + (ObjectWidth / 2) - 1, y + (ObjectWidth / 2) - 4 + (Wc / 2)); // ЗАКОДИТЬ НОРМАЛЬНО!

			QColor qc(240, 240, 240);
			if(c!=qc)
				qc=c;
			
			painter.setPen(QPen(c /*QColor(240, 240, 240)*/, 1));
			const QPointF pf3(x + (ObjectWidth / 2) - 1 + 0.5, y + (ObjectWidth / 2) - 4 + (Wc / 2) + 0.5);
			painter.drawPoint(pf3);
		};

		const Ship& s = _screenObjects[masNumber];

		const auto drawX = [&painter](const int x, const int y, const QColor& color)
		{
			painter.setPen(QPen(color, 3));
			painter.setFont(DRAW_FONT);
			painter.drawText(x + 3, y + (ObjectWidth - 4), "X");
		};

		const bool isDeny = IsDenyNearBeat(masNumber);
		static const QColor WeakHitCol = QColor(0xff, 0x7e, 0x67);
		static const QColor WeakCol = Qt::darkGreen;
		static constexpr int Margin = BigMargin - MarginX;
		if (const int dx = Margin + x; s.GetBeat(Ship::BEAT::ME))
		{
			if (const QColor c = _lastHitMy == masNumber ? Qt::red : (isDeny ? WeakCol : WeakHitCol); s.GetHolding(Ship::HOLDING::RIVAL))
				drawX(dx, y, c);
			else
				drawBall(dx, y, c);
		}
		else
			if (isDeny)
				drawBall(dx, y, Qt::blue);
			else
				if (dr && !s.GetBeat(Ship::BEAT::ME) && s.GetHolding(Ship::HOLDING::RIVAL))
					drawX(dx, y, Qt::black);
		if (const QColor c = _lastHitRival == masNumber ? Qt::red : WeakHitCol; s.GetBeat(Ship::BEAT::RIVAL))
		{
			if (s.GetHolding(Ship::HOLDING::ME))
				drawX(x, y, c);
			else
				drawBall(x, y, c);
		}
	};

	const auto draw = [&drawShipAndFrame](const int x, const int y, const int mx, const int my, const int w, const int h, const Ship& s, const bool drawRival)
	{
		if (s.GetHolding(Ship::HOLDING::ME))
			drawShipAndFrame(x, y, mx, my, w, h, s, drawRival);
	};

	for (int mx = 0, x = MarginX; mx < 10; ++mx, x += ObjectWidth)
		for (int my = 0, y = MarginY; my < 10; ++my, y += ObjectWidth)
			mBeat(x, y, (my * 10) + mx);

	drawField();

	for (int mx = 0, x = 0; mx < 10; ++mx, x += ObjectWidth)
		for (int my = 0, y = MarginY; my < 10; ++my, y += ObjectWidth)
		{
			const Ship& s = _screenObjects[(my * 10) + mx];
			const Ship::ROTATE r = s.GetRotate();
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

	if (const bool dc = ConnectionStatus == CONNECTIONSTATUS::CONNECTED; (ConnectionStatus == CONNECTIONSTATUS::DISCONNECTED && dr) || dc)
	{
		bool shipCoords[100] = { false };
		static constexpr int Margin = BigMargin - MarginX;

		for (int mx = 0, x = Margin; mx < 10; ++mx, x += ObjectWidth)
			for (int my = 0, y = MarginY; my < 10; ++my, y += ObjectWidth)
			{
				const Ship s = GetRivalShip((my * 10) + mx, shipCoords, dc);
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

	if (ConnectionStatus != CONNECTIONSTATUS::CONNECTED)
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
	_isDrawRivals = false;
}

void Graphics::DrawRivals()
{
	_isDrawRivals = true;
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
