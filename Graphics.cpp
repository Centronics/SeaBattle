#include "stdafx.h"
#include "Packet.h"
#include "Graphics.h"

using namespace std;

static const QFont DRAW_FONT("Times", Graphics::ObjectWidth - 4, QFont::Bold);
static const QFont TEXT_FONT("Times", 13);
static const QFont BIG_FONT("Arial", 15, QFont::Bold);
static const QPen DRAW_TEXT_PEN(Qt::red, 3);

void Graphics::Paint(QPainter& painter, const Ship::TYPES ship, const Ship::ROTATE rotate) const
{
	DrawField(painter);
	DrawShips(painter, ship, rotate);
}

void Graphics::DrawMoveQuad(QPainter& painter)
{
	if (IsShipAddition)
		return;

	static const QPen Bp(Qt::black, BetweenObjects);
	static const QPen Rp(Qt::red, BetweenObjects);
	static const QBrush Rb(Qt::red, Qt::SolidPattern);
	static const QPen Gp(Qt::green, BetweenObjects);
	static const QBrush Gb(Qt::green, Qt::SolidPattern);
	static const QPen Rpen(Qt::red, 3);
	static const QPen Gpen(Qt::green, 3);

	if (ConnectingStatus == CONNECTINGSTATUS::SERVER || ConnectingStatus == CONNECTINGSTATUS::CLIENT)
	{
		painter.setPen(Rpen);
		painter.setFont(BIG_FONT);
		painter.drawText(341, 285, ConnectingStatus == CONNECTINGSTATUS::SERVER ? "Ожидание подключения." : "Подключение...");
		return;
	}

	if (IsRivalMove)
	{
		painter.setPen(Rpen);
		painter.setFont(TEXT_FONT);
		painter.drawText(406, 250, "Ход соперника.");
		painter.setPen(Rp);
		painter.setBrush(Rb);
	}
	else
	{
		painter.setPen(Gpen);
		painter.setFont(TEXT_FONT);
		painter.drawText(432, 250, "Ваш ход.");
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
	if (!IsShipAddition)
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
	if (!IsShipAddition)
		return SHIPADDITION::INCORRECTMODE;
	if (AddOrRemove(get<1>(xd), get<2>(xd), Ship::TYPES::EMPTY, get<3>(xd)) != SHIPADDITION::OK)
		throw exception(__func__);
	return SHIPADDITION::OK;
}

bool Graphics::RivalHit(const quint8 coord)
{
	Ship& ship = _screenObjects.at(coord);
	if (ship.GetBeat(Ship::BEAT::ME))
		ship.SetBit(Ship::BIT::BOTH);
	return ship.GetHolding(Ship::HOLDING::ME);
}

bool Graphics::MyHit(const quint8 coord)
{
	Ship& ship = _screenObjects.at(coord);
	if (ship.GetBeat(Ship::BEAT::RIVAL))
		ship.SetBit(Ship::BIT::BOTH);
	return ship.GetHolding(Ship::HOLDING::RIVAL);
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
	if (CursorX < Margin || CursorX >= MaxCoord || CursorY < Margin || CursorY >= MaxCoord)
		return make_tuple(false, -1, -1);
	int mx = CursorX - Margin, my = CursorY - Margin;
	mx = mx - (mx % ObjectWidth);
	my = my - (my % ObjectWidth);
	mx += Margin;
	my += Margin;
	return make_tuple(true, mx, my);
}

tuple<bool, int, int> Graphics::GetMassiveCoords()
{
	if (CursorX < Margin || CursorX >= MaxCoord || CursorY < Margin || CursorY >= MaxCoord)
		return make_tuple(false, -1, -1);
	int mx = CursorX - Margin, my = CursorY - Margin;
	mx = mx - (mx % ObjectWidth);
	my = my - (my % ObjectWidth);
	mx /= ObjectWidth;
	my /= ObjectWidth;
	return make_tuple(true, mx, my);
}

tuple<bool, int, int, Ship::ROTATE> Graphics::GetShipCoords() const
{
	const optional<quint8> mas = GetCoord();
	if (CursorX < Margin || CursorX >= MaxCoord || CursorY < Margin || CursorY >= MaxCoord || !mas)
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

void Graphics::DrawWarning(QPainter& painter)
{
	painter.setPen(DRAW_TEXT_PEN);
	painter.setFont(DRAW_FONT);
	painter.drawText(CursorX - 12, CursorY + 20, "!");
}

void Graphics::DrawShips(QPainter& painter, const Ship::TYPES ship, const Ship::ROTATE rotate) const
{
	const auto drawShipAndFrame = [&painter, ship, rotate, this](const int x, const int y, const int mx, const int my, const int w, const int h, const Ship& s)
	{
		static constexpr int W = BetweenObjects * 2;
		static const QPen G(Qt::gray, BetweenObjects);
		static const QBrush D(Qt::gray, Qt::Dense6Pattern);
		const int xw = x + w, yh = y + h;
		const auto grey = [&painter] { painter.setPen(G); painter.setBrush(D); };
		const auto inFrame = CursorX >= x && CursorX < (x + ObjectWidth) && CursorY >= y && CursorY < (y + ObjectWidth);
		const auto drawShip = [x, y, w, h, &painter] { painter.drawRect(x - BetweenObjects, y - BetweenObjects, w + W, h + W); };
		const auto drawMark = [&painter, &grey](const int px, const int py) { grey(); painter.drawRect(px - BetweenObjects, py - BetweenObjects, ObjectWidth + W, ObjectWidth + W); };
		const bool inShip = CursorX >= x && CursorX < xw && CursorY >= y && CursorY < yh;

		if ((xw > MaxCoord || yh > MaxCoord) && inShip && inFrame)
		{
			drawMark(x, y);
			if (ship != Ship::TYPES::EMPTY && rotate != Ship::ROTATE::NIL)
				DrawWarning(painter);
			return;
		}

		if (s.GetRotate() != Ship::ROTATE::NIL)
		{
			const QColor color = Ship::GetColor(s.GetShipType());
			painter.setPen(QPen(color, BetweenObjects));
			painter.setBrush(QBrush(color, Qt::Dense6Pattern));
			drawShip();
			if (!IsRivalMove && inShip && inFrame)
				drawMark(x, y);
		}

		if (!IsRivalMove && inFrame)
		{
			if (!inShip || s.GetShipType() == Ship::TYPES::EMPTY)
			{
				grey();
				drawShip();
			}
			else
				drawMark(x, y);
		}

		if (!IsRivalMove && inFrame && IsReadyToPlay())
			drawMark(x, y);

		if (inFrame && IsBusy(mx, my, ship, rotate))
			DrawWarning(painter);
	};

	const auto mBeat = [&painter, &drawShipAndFrame](const int x, const int y, const int mx, const int my, const Ship::BIT bit, const Ship& s)
	{
		switch (bit)
		{
		case Ship::BIT::ME:
		{
			if (IsRivalMove)
				return;
			static const QPen Cpen(Qt::green, BetweenObjects);
			painter.setPen(Cpen);
			static constexpr int W = ObjectWidth / 2;
			painter.drawPoint(x + W, y + W);
			drawShipAndFrame(x, y, mx, my, ObjectWidth, ObjectWidth, s);
			return;
		}
		case Ship::BIT::RIVAL:
		{
			if (!IsRivalMove)
				return;
			painter.setPen(DRAW_TEXT_PEN);
			painter.setFont(DRAW_FONT);
			painter.drawText(x + 4, y + (ObjectWidth - 2), "X");
			return;
		}
		case Ship::BIT::NIL:
			return;
		default:
			throw exception("DrawShipRect(mBeat)");
		}
	};

	const auto draw = [&drawShipAndFrame](const int x, const int y, const int mx, const int my, const int w, const int h, const Ship& s)
	{
		if (!IsRivalMove && s.GetHolding(Ship::HOLDING::RIVAL) && s.GetBeat(Ship::BEAT::ME))
			drawShipAndFrame(x, y, mx, my, ObjectWidth, ObjectWidth, s);
		if ((IsRivalMove || IsShipAddition) && s.GetHolding(Ship::HOLDING::ME))
			drawShipAndFrame(x, y, mx, my, w, h, s);
	};

	for (int mx = 0, x = Margin; mx < 10; ++mx, x += ObjectWidth)
		for (int my = 0, y = Margin; my < 10; ++my, y += ObjectWidth)
		{
			const Ship& s = _screenObjects[(my * 10) + mx];
			mBeat(x, y, mx, my, s.GetBit(), s);
			switch (const int floors = Ship::GetFloors(s.GetShipType()) * ObjectWidth; s.GetRotate())
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
	if (!IsShipAddition)
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

void Graphics::DrawField(QPainter& painter)
{
	const auto proc = [&painter](const int x, const int y, const int mx, const int my)
	{
		static const QPen Penb(Qt::blue, BetweenObjects);
		painter.setPen(Penb);
		painter.setRenderHint(QPainter::Antialiasing);
		painter.drawLine(x, y, mx, my);
	};

	for (int x = 0, yc = Margin, xc = Margin + (10 * ObjectWidth); x < 11; ++x)
	{
		proc(Margin, yc, xc, yc);
		yc += ObjectWidth;
	}
	for (int y = 0, xc = Margin, yc = Margin + (10 * ObjectWidth); y < 11; ++y)
	{
		proc(xc, Margin, xc, yc);
		xc += ObjectWidth;
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
