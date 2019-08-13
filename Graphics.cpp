#include "stdafx.h"
#include "Packet.h"
#include "Graphics.h"

using namespace std;

static const QFont DRAW_FONT("Times", Graphics::ObjectWidth - 4, QFont::Bold);
static const QFont TEXT_FONT("Times", 13);
static const QPen DRAW_TEXT_PEN(Qt::red, 3);

void Graphics::Paint(QPainter& painter, const Ship::TYPES ship, const Ship::ROTATE rotate) const
{
	DrawField(painter);
	DrawShipRect(painter, ship, rotate);
	SetMoveQuad(painter);
	Clicked = false;
}

void Graphics::ClearRivalState()
{
	for (int x = 0; x < 10; ++x)
		for (int y = 0; y < 10; ++y)
			if (Ship& ship = _screenObjects[(y * 10) + x]; ship.GetShipHolder() == Ship::HOLDER::RIVAL)
				ship.Delete();
}

void Graphics::ClearField()
{
	for_each(_screenObjects.begin(), _screenObjects.end(), [](Ship& ship) { ship.Delete(); });
}

void Graphics::SetMoveQuad(QPainter& painter)
{
	if (ShipAddition)
		return;

	static const QPen Bp(Qt::black, BetweenObjects);
	static const QPen Rp(Qt::red, BetweenObjects);
	static const QBrush Rb(Qt::red, Qt::SolidPattern);
	static const QPen Gp(Qt::green, BetweenObjects);
	static const QBrush Gb(Qt::green, Qt::SolidPattern);
	static const QPen Rpen(Qt::red, 3);
	static const QPen Gpen(Qt::green, 3);

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
	if (!ShipAddition)
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
	if (!ShipAddition)
		return SHIPADDITION::INCORRECTMODE;
	const auto xs = GetMassiveCoords();
	if (!get<0>(xs))
		return SHIPADDITION::NOCOORD;
	if (AddOrRemove(get<1>(xs), get<2>(xs), Ship::TYPES::EMPTY, Ship::ROTATE::NIL) != SHIPADDITION::OK)
		throw exception(__func__);
	return SHIPADDITION::OK;
}

void Graphics::RivalHit(const quint8 coord)
{
	Ship& ship = _screenObjects.at(coord);
	if (ship.GetBeat(Ship::BEAT::ME))
		ship.SetBit(Ship::BIT::BOTH);
}

void Graphics::MyHit(const quint8 coord)
{
	Ship& ship = _screenObjects.at(coord);
	if (ship.GetBeat(Ship::BEAT::RIVAL))
		ship.SetBit(Ship::BIT::BOTH);
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

Graphics::BROKEN Graphics::IsBroken() const
{
	const auto broken = [this](const Ship::BIT bit)
	{
		const Ship::HOLDING h = bit == Ship::BIT::ME ? Ship::HOLDING::ME : Ship::HOLDING::RIVAL;
		for (unsigned int k = 0; k < _screenObjects.size(); ++k)
			if (_screenObjects[k].GetHolding(h) && !IsKilled(k, bit))
				return false;
		return true;
	};

	if (broken(Ship::BIT::ME))
		return BROKEN::ME;
	if (broken(Ship::BIT::RIVAL))
		return BROKEN::RIVAL;
	return BROKEN::NOTHING;
}

bool Graphics::IsKilled(quint8 coord, const Ship::BIT bit) const
{
	switch (const Ship& ship = _screenObjects[coord]; ship.GetRotate())
	{
	case Ship::ROTATE::STARTRIGHT:
		for (const unsigned int max = coord + Ship::GetFloors(ship.GetShipType()); coord < max; ++coord)
			if (bit == Ship::BIT::ME ? ship.GetBeat(Ship::BEAT::ME) : ship.GetBeat(Ship::BEAT::RIVAL))
				return false;
		return true;
	case Ship::ROTATE::STARTDOWN:
		for (const unsigned int max = coord + (Ship::GetFloors(ship.GetShipType()) * 10); coord < max; coord += 10)
			if (_screenObjects[coord].GetBit() != bit)
				return false;
		return true;
	default:
		return true;
	}
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

void Graphics::DrawShipRect(QPainter& painter, const Ship::TYPES ship, const Ship::ROTATE rotate) const
{
	const auto drawShipAndFrame = [&painter](const int x, const int y, const Ship* const pShip, const int w, const int h)
	{
		static constexpr int W = BetweenObjects * 2;
		static const QPen G(Qt::gray, BetweenObjects);
		static const QBrush D(Qt::gray, Qt::Dense6Pattern);
		const int xw = x + w, yh = y + h;
		if (xw > MaxCoord || yh > MaxCoord)
		{
			painter.setPen(DRAW_TEXT_PEN);
			painter.setFont(DRAW_FONT);
			painter.drawText(CursorX - 12, CursorY + 20, "!");
			return;
		}
		if (ShipAddition && CursorX >= x && CursorX < xw && CursorY >= y && CursorY < yh)
		{
			painter.setPen(G);
			painter.setBrush(D);
		}
		else
		{
			const QColor color = Ship::GetColor(pShip->GetShipType());
			painter.setPen(QPen(pShip ? color : Qt::black, BetweenObjects));
			painter.setBrush(QBrush(color, Qt::Dense6Pattern));
		}
		painter.drawRect(x - BetweenObjects, y - BetweenObjects, w + W, h + W);
	};

	const auto mBeat = [&painter, &drawShipAndFrame](const int x, const int y, const Ship::BIT bit)
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
			drawShipAndFrame(x, y, nullptr, ObjectWidth, ObjectWidth);
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
			throw exception("DrawShipState");
		}
	};

	for (int x = 0, xc = Margin; x < 10; ++x, xc += ObjectWidth)
		for (int y = 0, yc = Margin; y < 10; ++y, yc += ObjectWidth)
		{
			const Ship& s = _screenObjects[(y * 10) + x];
			mBeat(xc, yc, s.GetBit());
			const int floors = Ship::GetFloors(s.GetShipType()) * ObjectWidth;
			int tx, ty;
			switch (s.GetRotate())
			{
				int x2, y2;
			case Ship::ROTATE::NIL:
			{
				if (ship == Ship::TYPES::EMPTY || rotate == Ship::ROTATE::NIL)
					continue;
				const auto phs = GetPhysicalCoords();
				if (!get<0>(phs))
					continue;
				switch (const int fls = Ship::GetFloors(ship) * ObjectWidth; rotate)
				{
				case Ship::ROTATE::STARTRIGHT:
					x2 = fls;
					y2 = ObjectWidth;
					break;
				case Ship::ROTATE::STARTDOWN:
					x2 = ObjectWidth;
					y2 = fls;
					break;
				default:
					throw exception(__func__);
				}
				if (x2 <= MaxCoord && y2 <= MaxCoord)
					drawShipAndFrame(get<1>(phs), get<2>(phs), nullptr, x2, y2);
				continue;
			}
			case Ship::ROTATE::STARTRIGHT:
				tx = floors;
				ty = ObjectWidth;
				break;
			case Ship::ROTATE::STARTDOWN:
				tx = ObjectWidth;
				ty = floors;
				break;
			default:
				throw exception(__func__);
			}
			if (!IsRivalMove && s.GetHolding(Ship::HOLDING::RIVAL) && s.GetBeat(Ship::BEAT::ME))
				drawShipAndFrame(xc, yc, nullptr, ObjectWidth, ObjectWidth);
			if ((IsRivalMove || ShipAddition) && s.GetHolding(Ship::HOLDING::ME))
				drawShipAndFrame(xc, yc, &s, tx, ty);
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
		return !_screenObjects[static_cast<unsigned int>((y * 10) + x)].GetHolding(Ship::HOLDING::ME);
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

Graphics::SHIPADDITION Graphics::AddOrRemove(const int startX, const int startY, const Ship::TYPES ship, const Ship::ROTATE rotate)
{
	if (!ShipAddition)
		return SHIPADDITION::INCORRECTMODE;

	if (_screenObjects.size() != 100 || startX < 0 || startX > 9 || startY < 0 || startY > 9)
		throw exception(__func__);

	const auto fSet = [](Ship& obj, const Ship::ROTATE rotate, const Ship::TYPES sp)
	{
		obj.SetRotate(rotate);
		obj.SetBit(Ship::BIT::NIL);
		obj.SetShipHolder(Ship::HOLDER::ME);
		obj.SetShipType(sp);
	};

	auto i = static_cast<unsigned int>((startY * 10) + startX);

	switch (rotate)
	{
	case Ship::ROTATE::STARTDOWN:
	{
		int max = startY + Ship::GetFloors(ship);
		if (max > 10)
			return SHIPADDITION::NOCOORD;
		if (ship != Ship::TYPES::EMPTY)
		{
			for (int k = startY; k < max; ++k)
				if (!IsFree(startX, k))
					return SHIPADDITION::NOTFREE;
		}
		else
			if ((max = Ship::GetFloors(_screenObjects[i].GetShipType())) == 0)
				return SHIPADDITION::NOSHIP;
		for (int k = startY; k < max; ++k, i += 10u)
			if (ship != Ship::TYPES::EMPTY)
				fSet(_screenObjects[i], k == startY ? Ship::ROTATE::STARTDOWN : Ship::ROTATE::NIL, ship);
			else
				_screenObjects[i].Delete();
		return SHIPADDITION::OK;
	}
	case Ship::ROTATE::STARTRIGHT:
	{
		int max = startX + Ship::GetFloors(ship);
		if (max > 10)
			return SHIPADDITION::NOCOORD;
		if (ship != Ship::TYPES::EMPTY)
		{
			for (int k = startX; k < max; ++k)
				if (!IsFree(k, startY))
					return SHIPADDITION::NOTFREE;
		}
		else
			if ((max = Ship::GetFloors(_screenObjects[i].GetShipType())) == 0)
				return SHIPADDITION::NOSHIP;
		for (int k = startX; k < max; ++k, ++i)
			if (ship != Ship::TYPES::EMPTY)
				fSet(_screenObjects[i], k == startX ? Ship::ROTATE::STARTRIGHT : Ship::ROTATE::NIL, ship);
			else
				_screenObjects[i].Delete();
		return SHIPADDITION::OK;
	}
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
		if (const Ship::ROTATE rotate = obj.GetRotate(); obj.GetShipType() == ship && (rotate == Ship::ROTATE::STARTRIGHT || rotate == Ship::ROTATE::STARTDOWN))
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

bool Graphics::ReadRivals(const Packet& packet)
{
	return packet.ReadRivals(_screenObjects);
}
