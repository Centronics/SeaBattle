#include "stdafx.h"
#include "Graphics.h"

using namespace std;

static const QFont DRAW_FONT("Times", Graphics::ObjectWidth - 4, QFont::Bold);

void Graphics::Paint(QPainter& painter, const Ship::SHIPS ship, const Ship::STATE state) const
{
	DrawField(painter);
	const int cx = CursorX, cy = CursorY;
	for (int x = 0; x < MaxCoord; x += ObjectWidth)
		for (int y = 0; y < MaxCoord; y += ObjectWidth)
		{
			CursorX = x;
			CursorY = y;
			DrawShipState(painter);
		}
	CursorX = cx;
	CursorY = cy;
	DrawFrame(painter, ship, state);
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

bool Graphics::AddShip(const Ship::SHIPS ship, const Ship::STATE state)
{
	if (IsReady() || !ShipAddition || ship == Ship::SHIPS::EMPTY)
		return false;
	const auto xs = GetMassiveCoords();
	if (!get<0>(xs))
		return false;
	return AddOrRemove(get<1>(xs), get<2>(xs), ship, state);
}

void Graphics::RemoveShip()
{
	if (!ShipAddition)
		return;
	const auto xs = GetMassiveCoords();
	if (!get<0>(xs))
		return;
	Q_UNUSED(AddOrRemove(get<1>(xs), get<2>(xs)));
}

bool Graphics::IsReady(const Ship::SHIPS ship) const
{
	if (ship == Ship::SHIPS::EMPTY)
		return Ship::GetMaxShipCount(Ship::SHIPS::CRUISER) == GetShipCount(Ship::SHIPS::CRUISER) &&
		Ship::GetMaxShipCount(Ship::SHIPS::ESMINEC) == GetShipCount(Ship::SHIPS::ESMINEC) &&
		Ship::GetMaxShipCount(Ship::SHIPS::LINKOR) == GetShipCount(Ship::SHIPS::LINKOR) &&
		Ship::GetMaxShipCount(Ship::SHIPS::VEDETTE) == GetShipCount(Ship::SHIPS::VEDETTE);
	return Ship::GetMaxShipCount(ship) == GetShipCount(ship);
}

void Graphics::DrawFrame(QPainter& painter, const Ship::SHIPS ship, const Ship::STATE state) const
{
	const auto drawNow = [&painter](const int x, const int y, const bool tempSelect = true, const int x1 = -1, const int y1 = -1)
	{
		static constexpr int Wh = ObjectWidth + (BetweenObjects * 2);
		static const QPen P(Qt::red, BetweenObjects);
		static const QPen G(Qt::gray, BetweenObjects);
		painter.setPen(tempSelect ? G : P);
		static const QBrush B(Qt::red, Qt::Dense6Pattern);
		static const QBrush D(Qt::gray, Qt::Dense6Pattern);
		painter.setBrush(tempSelect ? D : B);
		if (x1 < 0 || y1 < 0)
			painter.drawRect(x - BetweenObjects, y - BetweenObjects, Wh, Wh);
		else
			painter.drawRect(x - BetweenObjects, y - BetweenObjects, x1 + BetweenObjects, y1 + BetweenObjects);
	};

	const auto drawFrame = [&drawNow]
	{
		if (const auto res = GetPhysicalCoords(); get<0>(res))
			drawNow(get<1>(res), get<2>(res));
	};

	if (!ShipAddition)
	{
		drawFrame();
		return;
	}
	if (const auto sr = GetShipRect(false, ship, state); get<0>(sr))
		drawNow(get<1>(sr), get<2>(sr), false, get<3>(sr), get<4>(sr));
	else
		drawFrame();
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

void Graphics::DrawShipState(QPainter& painter) const
{
	const optional<quint8> masCoord = GetCoord();
	if (!masCoord)
		return;
	const Ship& pShip = _screenObjects[*masCoord];
	const Ship::SHIPS ship = pShip.GetShip();
	if (ship == Ship::SHIPS::EMPTY)
		return;

	const auto mBeat = [&painter, &pShip](const int cursorX, const int cursorY) -> bool
	{
		switch (pShip.GetBit())
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
		default:
			break;
		}
		return false;
	};

	switch (const Ship::STATE state = pShip.GetState())
	{
	case Ship::STATE::STARTRIGHT:
	case Ship::STATE::STARTDOWN:
	{
		const int floors = Ship::GetFloors(ship);
		auto shipRect = GetShipRect(true, Ship::SHIPS::EMPTY, Ship::STATE::NIL);
		if (!get<0>(shipRect))
			throw exception(__func__);
		int deadFloors = 0;
		switch (state)
		{
		case Ship::STATE::STARTRIGHT:
			for (int x = get<1>(shipRect), k = 0, y = get<2>(shipRect); k < floors; k++, x += ObjectWidth)
				deadFloors += mBeat(x, y);
			break;
		case Ship::STATE::STARTDOWN:
			for (int x = get<1>(shipRect), k = 0, y = get<2>(shipRect); k < floors; k++, y += ObjectWidth)
				deadFloors += mBeat(x, y);
			break;
		case Ship::STATE::NIL:
		default:
			break;
		}
		if (floors != deadFloors || IsRivalMove || pShip.GetHolder() != Ship::SHIPHOLDER::ME)
			return;
		const QColor col = Ship::GetColor(ship);
		painter.setPen(QPen(col, BetweenObjects * 2));
		painter.setBrush(QBrush(col, Qt::Dense6Pattern));
		painter.drawRect(get<1>(shipRect), get<2>(shipRect), get<3>(shipRect), get<4>(shipRect));
	}
	case Ship::STATE::NIL:
	default:
		break;
	}
}

tuple<bool, int, int, int, int> Graphics::GetShipRect(const bool realObject, const Ship::SHIPS ship, const Ship::STATE state) const
{
	const auto inRange = [](const int x1, const int y1, const int x2, const int y2) -> bool
	{
		return CursorX >= x1 && CursorX < x2 && CursorY >= y1 && CursorY < y2;
	};
	const auto phs = GetPhysicalCoords();
	if (!get<0>(phs))
		return make_tuple(false, -1, -1, -1, -1);
	const auto pms = GetMassiveCoords();
	if (!get<0>(pms) && realObject)
		return make_tuple(false, -1, -1, -1, -1);
	if (!realObject)
	{
		if (ship == Ship::SHIPS::EMPTY || state == Ship::STATE::NIL)
			return make_tuple(false, -1, -1, -1, -1);
		switch (const int fls = Ship::GetFloors(ship) * ObjectWidth; state)
		{
		case Ship::STATE::STARTRIGHT:
		{
			const int x2 = get<1>(phs) + fls, y2 = get<2>(phs) + ObjectWidth;
			if (x2 > MaxCoord || y2 > MaxCoord)
				return make_tuple(false, -1, -1, -1, -1);
			return make_tuple(true, get<1>(phs), get<2>(phs), x2, y2);
		}
		case Ship::STATE::STARTDOWN:
		{
			const int x2 = get<1>(phs) + ObjectWidth, y2 = get<2>(phs) + fls;
			if (x2 > MaxCoord || y2 > MaxCoord)
				return make_tuple(false, -1, -1, -1, -1);
			return make_tuple(true, get<1>(phs), get<2>(phs), x2, y2);
		}
		case Ship::STATE::NIL:
		default:
			return make_tuple(false, -1, -1, -1, -1);
		}
	}
	const Ship& obj = _screenObjects[static_cast<unsigned int>((get<2>(pms) * 10) + get<1>(pms))];
	const int fls = Ship::GetFloors(obj.GetShip()) * ObjectWidth;
	switch (obj.GetState())
	{
	case Ship::STATE::STARTRIGHT:
		return make_tuple(inRange(get<1>(phs), get<2>(phs), get<1>(phs) + fls, get<2>(phs) + ObjectWidth), get<1>(phs), get<2>(phs), fls, ObjectWidth);
	case Ship::STATE::STARTDOWN:
		return make_tuple(inRange(get<1>(phs), get<2>(phs), get<1>(phs) + ObjectWidth, get<2>(phs) + fls), get<1>(phs), get<2>(phs), ObjectWidth, fls);
	case Ship::STATE::NIL:
		return make_tuple(false, -1, -1, -1, -1);
	default:
		throw exception(__func__);
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
		return _screenObjects[static_cast<unsigned int>((y * 10) + x)].GetShip() == Ship::SHIPS::EMPTY;
	};

	bool result[]{
		coord(sx - 1, sy),
		coord(sx + 1, sy),
		coord(sx, sy - 1),
		coord(sx, sy + 1),
		coord(sx, sy) };

	static constexpr auto Size = sizeof(result) / sizeof(bool);
	return all_of(result, &result[Size], [](const bool b) { return b; });
}

bool Graphics::AddOrRemove(const int startX, const int startY, Ship::SHIPS ship, const Ship::STATE state)
{
	if (_screenObjects.size() != 100 || startX < 0 || startX > 9 || startY < 0 || startY > 9)
		throw exception(__func__);

	const auto fSet = [](Ship& obj, const Ship::STATE state, const Ship::SHIPS sp)
	{
		obj.SetState(state);
		obj.SetBit(Ship::BIT::NIL);
		obj.SetShip(sp);
	};

	const bool needDelete = ship == Ship::SHIPS::EMPTY;
	auto i = static_cast<unsigned int>((startY * 10) + startX);
	if (needDelete)
	{
		ship = _screenObjects[i].GetShip();
		if (ship == Ship::SHIPS::EMPTY)
			return true;
	}

	switch (state)
	{
	case Ship::STATE::STARTDOWN:
	{
		const int max = startY + Ship::GetFloors(ship);
		if (max > 10)
			return false;
		for (int k = startY; k < max; ++k)
			if (!IsFree(startX, k))
				return false;
		for (int k = startY; k < max; ++k, i += 10u)
			if (needDelete)
				_screenObjects[i].Delete();
			else
				fSet(_screenObjects[i], k == startY ? Ship::STATE::STARTDOWN : Ship::STATE::NIL, ship);
		return true;
	}
	case Ship::STATE::STARTRIGHT:
	{
		const int max = startX + Ship::GetFloors(ship);
		if (max > 10)
			return false;
		for (int k = startX; k < max; ++k)
			if (!IsFree(startX, k))
				return false;
		for (int k = startX; k < max; ++k, ++i)
			if (needDelete)
				_screenObjects[i].Delete();
			else
				fSet(_screenObjects[i], k == startX ? Ship::STATE::STARTRIGHT : Ship::STATE::NIL, ship);
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
	if (ship == Ship::SHIPS::EMPTY)
		return 0;
	int result = 0;
	for (const auto& obj : _screenObjects)
		if (const Ship::STATE state = obj.GetState(); obj.GetShip() == ship && (state == Ship::STATE::STARTRIGHT || state == Ship::STATE::STARTDOWN))
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

const vector<Ship>& Graphics::GetData() const
{
	return _screenObjects;
}
