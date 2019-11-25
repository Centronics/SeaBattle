#include "stdafx.h"
#include "Packet.h"
#include "Graphics.h"

using namespace std;

static const QFont DRAW_FONT_MARK("Courier New", Graphics::ObjectWidth - 5);
static const QFont DRAW_FONT("Times", Graphics::ObjectWidth - 5, QFont::Bold);
static const QFont TEXT_FONT("Times", 13);
static const QFont BIG_FONT("Arial", 15, QFont::Bold);
static const QPen DRAW_TEXT_PEN(Qt::red, 3);

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
			painter.drawText(341, 285, "Ожидание подключения.");
		if (ConnectionStatus == CONNECTIONSTATUS::CLIENT)
			painter.drawText(389, 285, "Подключение...");
		return;
	}

	if (IsRivalMove)
	{
		painter.setPen(Rpen);
		painter.setFont(TEXT_FONT);
		painter.drawText(411, 250, "Ход соперника.");
		painter.setPen(Rp);
		painter.setBrush(Rb);
	}
	else
	{
		painter.setPen(Gpen);
		painter.setFont(TEXT_FONT);
		painter.drawText(434, 250, "Ваш ход.");
		painter.setPen(Gp);
		painter.setBrush(Gb);
	}

	painter.drawEllipse(440, 265, 50, 50);
	painter.setPen(Bp);
	painter.setBrush(Qt::NoBrush);
	painter.drawRect(395, 230, 140, 100);
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
	if (!IsAllowNearBeat(coord))
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

tuple<bool, int, int> Graphics::GetPhysicalCoords()
{
	if (CursorX < MarginX || CursorX >= MaxCoordX || CursorY < MarginY || CursorY >= MaxCoordY)
		return make_tuple(false, -1, -1);
	int mx = CursorX - MarginX, my = CursorY - MarginY;
	mx = mx - (mx % ObjectWidth);
	my = my - (my % ObjectWidth);
	mx += MarginX;
	my += MarginY;
	return make_tuple(true, mx, my);
}

tuple<bool, int, int> Graphics::GetMassiveCoords()
{
	if (CursorX < MarginX || CursorX >= MaxCoordX || CursorY < MarginY || CursorY >= MaxCoordY)
		return make_tuple(false, -1, -1);
	int mx = CursorX - MarginX, my = CursorY - MarginY;
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
				if (mx >= x && mx < (x + Ship::GetFloors(s.GetShipType())) && my == y)
					return make_tuple(true, x, y, Ship::ROTATE::STARTRIGHT);
				continue;
			case Ship::ROTATE::STARTDOWN:
				if (my >= y && my < (y + Ship::GetFloors(s.GetShipType())) && mx == x)
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

const Ship* Graphics::IsKilled(const quint8 coord) const
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
}

bool Graphics::IsAllowNearBeat(const quint8 coord) const
{
	enum class VALUE
	{
		START,
		END,
		BOTH
	};

	const auto inRangeX = [coord](const quint8 x, const VALUE value)
	{
		if (value == VALUE::END || value == VALUE::BOTH)
			if (const quint8 ix = x + 1; (ix < 10) && (ix == coord))
				return true;
		if (value == VALUE::START || value == VALUE::BOTH)
			if ((x > 0) && ((x - 1) == coord))
				return true;
		return false;
	};

	const auto inRangeY = [coord](const quint8 y, const VALUE value)
	{
		if (value == VALUE::END || value == VALUE::BOTH)
			if (const quint8 iy = y + 1; (iy < 10) && (iy == coord))
				return true;
		if (value == VALUE::START || value == VALUE::BOTH)
			if ((y > 0) && ((y - 1) == coord))
				return true;
		return false;
	};

	for (quint8 n = 0; n < 100; ++n)
		if (const Ship* s = IsKilled(n))
			switch (const quint8 x = n % 10, y = n / 10; s->GetRotate())
			{
			case Ship::ROTATE::STARTRIGHT:
				if (inRangeX(x, VALUE::START))
					return false;
				for (quint8 j = 1, mj = s->GetFloors() - 1; j < mj; ++j)
					if (inRangeY(x, VALUE::BOTH))
						return false;
				if (inRangeX(x, VALUE::END))
					return false;
				continue;
			case Ship::ROTATE::STARTDOWN:
				if (inRangeY(y, VALUE::START))
					return false;
				for (quint8 j = 1, mj = s->GetFloors() - 1; j < mj; ++j)
					if (inRangeX(y, VALUE::BOTH))
						return false;
				if (inRangeY(y, VALUE::END))
					return false;
				continue;
			default:
				throw exception("drawKilledShips");
			}
	return true;
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
		const int floors = Ship::GetFloors(s->GetShipType());
		for (int k = 0; k < floors; ++k, s = (r == Ship::ROTATE::STARTDOWN) ? s + 10 : s + 1)
			if (s->GetBeat(Ship::BEAT::RIVAL))
				return true;
		return false;
	};

	const auto drawShipAndFrame = [&painter, ship, rotate, &xMarkCoord, &yMarkCoord, &wMarkCoord, &hMarkCoord, &grey, &drawWarning, this, &isShipBeat](const int x, const int y, const int mx, const int my, const int w, const int h, const Ship& s)
	{
		const int xw = x + w, yh = y + h;
		const auto inFrame = CursorX >= x && CursorX < (x + ObjectWidth) && CursorY >= y && CursorY < (y + ObjectWidth);
		const auto drawShip = [x, y, w, h, &xMarkCoord, &yMarkCoord, &wMarkCoord, &hMarkCoord, &painter](const bool now) { if (now) painter.drawRect(x - BetweenObjects, y - BetweenObjects, w + W, h + W); else { xMarkCoord = x; yMarkCoord = y; wMarkCoord = w; hMarkCoord = h; } };
		const bool inShip = CursorX >= x && CursorX < xw && CursorY >= y && CursorY < yh;
		const auto drawMark = [x, y, &xMarkCoord, &yMarkCoord] { xMarkCoord = x, yMarkCoord = y; };

		if ((xw > MaxCoordX || yh > MaxCoordY) && inShip && inFrame)
		{
			drawMark();
			if (ship != Ship::TYPES::EMPTY && rotate != Ship::ROTATE::NIL)
				drawWarning();
			return;
		}

		if (s.GetRotate() != Ship::ROTATE::NIL)
		{
			const QColor color = Ship::GetColor(s.GetShipType());
			painter.setPen(QPen(color, BetweenObjects));
			painter.setBrush(QBrush(color, isShipBeat(mx, my) ? Qt::NoBrush : Qt::Dense6Pattern));
			if (IsRivalMove || ConnectionStatus == CONNECTIONSTATUS::DISCONNECTED)
				drawShip(true);
			if (!IsRivalMove && inShip && inFrame)
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

	const auto mBeat = [&painter](const int x, const int y, const Ship& s)
	{
		if (ConnectionStatus != CONNECTIONSTATUS::CONNECTED)
			return;

		const auto drawX = [&painter](const int x, const int y)
		{
			painter.setPen(DRAW_TEXT_PEN);
			painter.setFont(DRAW_FONT);
			painter.drawText(x + 3, y + (ObjectWidth - 4), "X");
		};

		const auto drawBall = [&painter](const int x, const int y, const Qt::GlobalColor color)
		{
			static constexpr int Wd = ObjectWidth / 2;
			static constexpr int Wc = (Wd / 2);
			static const QPen B(color, Wd);
			painter.setPen(B);
			painter.drawEllipse(x + Wc + 3, y + Wc + 3, Wd - 6, Wd - 6);
			painter.drawPoint(x + Wd, y + Wd);
		};

		if (const Ship::BIT bit = s.GetBit(); (bit == Ship::BIT::ME || bit == Ship::BIT::BOTH) && !IsRivalMove)
		{
			if (s.GetHolding(Ship::HOLDING::RIVAL))
				drawX(x, y);
			else
				drawBall(x, y, Qt::black);
		}
		else
			if ((bit == Ship::BIT::RIVAL || bit == Ship::BIT::BOTH) && IsRivalMove && s.GetShipType() != Ship::TYPES::EMPTY)
				drawBall(x, y, Qt::red);
	};

	const auto drawKilledShips = [&painter, this]
	{
		if (ConnectionStatus != CONNECTIONSTATUS::CONNECTED)
			return;
		for (quint8 k = 0; k < 100; ++k)
			if (!IsAllowNearBeat(k))
			{
				static constexpr int Wd = ObjectWidth / 2;
				static constexpr int Wc = (Wd / 2);
				static const QPen B(Qt::red, Wd);
				const int x = k % 10, y = k / 10;
				painter.setPen(B);
				painter.drawEllipse(x + Wc + 3, y + Wc + 3, Wd - 6, Wd - 6);
				painter.drawPoint(x + Wd, y + Wd);
			}
	};

	const auto draw = [&drawShipAndFrame](const int x, const int y, const int mx, const int my, const int w, const int h, const Ship& s)
	{
		if (ConnectionStatus == CONNECTIONSTATUS::CONNECTED && !IsRivalMove && s.GetHolding(Ship::HOLDING::RIVAL) && s.GetBeat(Ship::BEAT::ME))
			drawShipAndFrame(x, y, mx, my, ObjectWidth, ObjectWidth, s);
		if ((IsRivalMove || ConnectionStatus == CONNECTIONSTATUS::DISCONNECTED) && s.GetHolding(Ship::HOLDING::ME))
			drawShipAndFrame(x, y, mx, my, w, h, s);
	};

	drawKilledShips();

	if (ConnectionStatus == CONNECTIONSTATUS::CONNECTED)
	{
		const auto drawMark = [&painter](const int x, const int y)
		{
			static const QColor HitColor = QColor(255, 168, 153);
			painter.setPen(QPen(HitColor, BetweenObjects));
			painter.setBrush(Qt::NoBrush);
			painter.drawRect(x - BetweenObjects, y - BetweenObjects, ObjectWidth + W, ObjectWidth + W);
		};

		if (_lastHitMy < 100)
		{
			const int x = ((_lastHitMy % 100) * ObjectWidth) + MarginX, y = ((_lastHitMy / 100) * ObjectWidth) + MarginY;
			drawMark(x, y);
		}
		if (_lastHitRival < 100)
		{
			const int x = ((_lastHitMy % 100) * ObjectWidth) + BigMargin, y = ((_lastHitMy / 100) * ObjectWidth) + MarginY;
			drawMark(x, y);
		}
	}
	else
	{
		_lastHitMy = 255;
		_lastHitRival = 255;
	}

	for (int mx = 0, x = MarginX; mx < 10; ++mx, x += ObjectWidth)
		for (int my = 0, y = MarginY; my < 10; ++my, y += ObjectWidth)
			mBeat(x, y, _screenObjects[(my * 10) + mx]);

	drawField();

	for (int mx = 0, x = MarginX; mx < 10; ++mx, x += ObjectWidth)
		for (int my = 0, y = MarginY; my < 10; ++my, y += ObjectWidth)
		{
			const Ship& s = _screenObjects[(my * 10) + mx];
			const Ship::ROTATE r = (!IsRivalMove && ConnectionStatus != CONNECTIONSTATUS::DISCONNECTED) ? Ship::ROTATE::NIL : s.GetRotate();
			switch (const int floors = Ship::GetFloors(s.GetShipType()) * ObjectWidth; r)
			{
			case Ship::ROTATE::STARTRIGHT:
				draw(x, y, mx, my, floors, ObjectWidth, s);
				continue;
			case Ship::ROTATE::STARTDOWN:
				draw(x, y, mx, my, ObjectWidth, floors, s);
				continue;
			case Ship::ROTATE::NIL:
				switch (const int fls = Ship::GetFloors(ship) * ObjectWidth; rotate)
				{
				case Ship::ROTATE::STARTRIGHT:
					drawShipAndFrame(x, y, mx, my, fls, ObjectWidth, s);
					continue;
				case Ship::ROTATE::STARTDOWN:
					drawShipAndFrame(x, y, mx, my, ObjectWidth, fls, s);
					continue;
				case Ship::ROTATE::NIL:
					drawShipAndFrame(x, y, mx, my, ObjectWidth, ObjectWidth, s);
					continue;
				default:
					throw exception(__func__);
				}
			default:
				throw exception(__func__);
			}
		}

	marking();

	if (dWarnX < 0 || dWarnY < 0)
		return;
	painter.setPen(DRAW_TEXT_PEN);
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
		if (const int c = Ship::GetFloors(_screenObjects[i].GetShipType()); c != 0)
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
