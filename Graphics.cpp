#include "stdafx.h"
#include "Graphics.h"

using namespace std;

static const QFont DRAW_FONT("Times", Graphics::ObjectWidth - 4, QFont::Bold);

void Graphics::Paint(QPainter& painter, const optional<Ship::SHIPS> ship, const Ship::ROTATE rotate) const
{
	DrawField(painter);
	DrawShipRect(painter, ship, rotate);
	Clicked = false;
}

void Graphics::ClearRivalState()
{
	for (int x = 0; x < 10; ++x)
		for (int y = 0; y < 10; ++y)
			if (Ship& ship = _screenObjects[(y * 10) + x]; ship.GetHolder() == Ship::SHIPHOLDER::RIVAL)
				ship.Delete();
}

void Graphics::ClearField()
{
	for_each(_screenObjects.begin(), _screenObjects.end(), [](Ship& ship) { ship.Delete(); });
}

bool Graphics::AddShip(const Ship::SHIPS ship, const Ship::ROTATE rotate)
{
	if (IsReady() || !ShipAddition)
		return false;
	const auto xs = GetMassiveCoords();
	if (!get<0>(xs))
		return false;
	return AddOrRemove(get<1>(xs), get<2>(xs), ship, rotate);
}

void Graphics::RemoveShip()
{
	if (!ShipAddition)
		return;
	const auto xs = GetMassiveCoords();
	if (!get<0>(xs))
		return;
	Q_UNUSED(AddOrRemove(get<1>(xs), get<2>(xs), nullopt, Ship::ROTATE::NIL));
}

bool Graphics::IsReady(const optional<Ship::SHIPS> ship) const
{
	if (!ship)
		return Ship::GetMaxShipCount(Ship::SHIPS::CRUISER) == GetShipCount(Ship::SHIPS::CRUISER) &&
		Ship::GetMaxShipCount(Ship::SHIPS::ESMINEC) == GetShipCount(Ship::SHIPS::ESMINEC) &&
		Ship::GetMaxShipCount(Ship::SHIPS::LINKOR) == GetShipCount(Ship::SHIPS::LINKOR) &&
		Ship::GetMaxShipCount(Ship::SHIPS::VEDETTE) == GetShipCount(Ship::SHIPS::VEDETTE);
	return Ship::GetMaxShipCount(*ship) == GetShipCount(*ship);
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

void Graphics::DrawShipRect(QPainter& painter, optional<const Ship::SHIPS> ship, const Ship::ROTATE rotate) const
{
	const auto inRange = [](const int x1, const int y1, const int x2, const int y2) -> bool
	{
		return CursorX >= x1 && CursorX < x2 && CursorY >= y1 && CursorY < y2;
	};

	const auto mBeat = [&painter](const int cursorX, const int cursorY, const Ship::BIT bit) -> bool
	{
		switch (bit)
		{
		case Ship::BIT::MYBEAT:
		{
			if (IsRivalMove)
				return false;
			static const QPen Cpen(Qt::green, BetweenObjects);
			painter.setPen(Cpen);
			static constexpr int W = ObjectWidth / 2;
			painter.drawPoint(cursorX + W, cursorY + W);
			return false;
		}
		case Ship::BIT::RIVALBEAT:
		{
			if (!IsRivalMove)
				return false;
			static const QPen Dpen(Qt::red, 3);
			painter.setPen(Dpen);
			painter.setFont(DRAW_FONT);
			painter.drawText(cursorX + 4, cursorY + (ObjectWidth - 2), "X");
			return true;
		}
		case Ship::BIT::NIL:
			return false;
		default:
			throw exception("DrawShipState");
		}
	};

	const auto drawMyShipAndFrame = [&painter](const int x, const int y, const bool tempSelect, const QColor& shipColor, const int w = -1, const int h = -1)
	{
		if (IsRivalMove)
			return;
		static constexpr int W = BetweenObjects * 2;
		static constexpr int Wh = ObjectWidth + W;
		static const QPen P(Qt::red, BetweenObjects);
		static const QPen G(Qt::gray, BetweenObjects);
		static const QBrush B(Qt::red, Qt::Dense6Pattern);
		static const QBrush D(Qt::gray, Qt::Dense6Pattern);
		if (w < 0 || h < 0)
		{
			painter.setPen(tempSelect ? G : P);
			painter.setBrush(tempSelect ? D : B);
			painter.drawRect(x - BetweenObjects, y - BetweenObjects, Wh, Wh);
			return;
		}
		painter.setPen(shipColor);
		painter.setBrush(Qt::NoBrush);
		painter.drawRect(x, y, w, h);
		painter.setPen(tempSelect ? G : P);
		painter.setBrush(tempSelect ? D : B);
		painter.drawRect(x - BetweenObjects, y - BetweenObjects, w + W, h + W);
	};

	const auto drawEnemyShip = [&painter, &mBeat](const int px, const int py, const int sizeX, const int sizeY, const Ship* pShip)
	{
		if (IsRivalMove)
			return;
		const int floors = Ship::GetFloors(pShip->GetShip());
		int deadFloors = 0;
		switch (pShip->GetRotate())
		{
		case Ship::ROTATE::STARTRIGHT:
			for (int x = px, k = 0, y = py; k < floors; k++, x += ObjectWidth)
				deadFloors += mBeat(x, y, pShip->GetBit());
			break;
		case Ship::ROTATE::STARTDOWN:
			for (int x = px, k = 0, y = py; k < floors; k++, y += ObjectWidth)
				deadFloors += mBeat(x, y, pShip->GetBit());
			break;
		case Ship::ROTATE::NIL:
		default:
			throw exception("GetShipRect");
		}
		if (floors != deadFloors)
			return;
		const QColor col = Ship::GetColor(pShip->GetShip());
		painter.setPen(QPen(col, BetweenObjects * 2));
		painter.setBrush(QBrush(col, Qt::Dense6Pattern));
		painter.drawRect(px, py, sizeX, sizeY);
	};

	for (int x = 0, p = 0, xc = Margin; x < 10; ++x, xc += ObjectWidth)
		for (int y = 0, yc = Margin; y < 10; ++y, ++p, yc += ObjectWidth)
		{
			const Ship& s = _screenObjects[p];
			if (!s.GetIsMyHolding())
				continue;
			const int floors = Ship::GetFloors(s.GetShip()) * ObjectWidth;
			int tx, ty;
			switch (s.GetRotate())
			{
				int x2, y2;
			case Ship::ROTATE::NIL:
			{
				if (!ShipAddition)
				{
					drawMyShipAndFrame(xc, yc, false, Qt::black);
					continue;
				}
				if (!ship || rotate == Ship::ROTATE::NIL)
					continue;
				const auto phs = GetPhysicalCoords();
				if (!get<0>(phs))
					continue;
				switch (const int fls = Ship::GetFloors(*ship) * ObjectWidth; rotate)
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
				if (x2 > MaxCoord || y2 > MaxCoord)
					continue;
				drawMyShipAndFrame(get<1>(phs), get<2>(phs), true, Ship::GetColor(*ship), x2, y2);
				continue;
			}
			case Ship::ROTATE::STARTRIGHT:
				tx = xc + floors;
				ty = yc + ObjectWidth;
				break;
			case Ship::ROTATE::STARTDOWN:
				tx = xc + ObjectWidth;
				ty = yc + floors;
				break;
			default:
				throw exception(__func__);
			}
			drawEnemyShip(xc, yc, tx, ty, &s);
			drawMyShipAndFrame(xc, yc, !inRange(xc, yc, tx, ty), Ship::GetColor(s.GetShip()), tx, ty);
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
		const Ship& ship = _screenObjects[static_cast<unsigned int>((y * 10) + x)];
		const Ship::SHIPHOLDER s = ship.GetHolder();
		return s != Ship::SHIPHOLDER::BOTH && s != Ship::SHIPHOLDER::ME;
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

	return true;
}

bool Graphics::AddOrRemove(const int startX, const int startY, const optional<Ship::SHIPS> ship, const Ship::ROTATE rotate)
{
	if (_screenObjects.size() != 100 || startX < 0 || startX > 9 || startY < 0 || startY > 9)
		throw exception(__func__);

	const auto fSet = [](Ship& obj, const Ship::ROTATE rotate, const Ship::SHIPS sp)
	{
		obj.SetRotate(rotate);
		obj.SetBit(Ship::BIT::NIL);
		obj.SetHolder(Ship::SHIPHOLDER::ME);
		obj.SetShip(sp);
	};

	auto i = static_cast<unsigned int>((startY * 10) + startX);

	switch (rotate)
	{
	case Ship::ROTATE::STARTDOWN:
	{
		const int max = startY + (ship ? Ship::GetFloors(*ship) : 0);
		if (max > 10)
			return false;
		for (int k = startY; k < max; ++k)
			if (!IsFree(startX, k))
				return false;
		for (int k = startY; k < max; ++k, i += 10u)
			if (ship)
				fSet(_screenObjects[i], k == startY ? Ship::ROTATE::STARTDOWN : Ship::ROTATE::NIL, *ship);
			else
				_screenObjects[i].Delete();
		return true;
	}
	case Ship::ROTATE::STARTRIGHT:
	{
		const int max = startX + (ship ? Ship::GetFloors(*ship) : 0);
		if (max > 10)
			return false;
		for (int k = startX; k < max; ++k)
			if (!IsFree(k, startY))
				return false;
		for (int k = startX; k < max; ++k, ++i)
			if (ship)
				fSet(_screenObjects[i], k == startX ? Ship::ROTATE::STARTRIGHT : Ship::ROTATE::NIL, *ship);
			else
				_screenObjects[i].Delete();
		return true;
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

int Graphics::GetShipCount(const Ship::SHIPS ship) const
{
	int result = 0;
	for (const auto& obj : _screenObjects)
		if (const Ship::ROTATE rotate = obj.GetRotate(); obj.GetShip() == ship && (rotate == Ship::ROTATE::STARTRIGHT || rotate == Ship::ROTATE::STARTDOWN))
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
