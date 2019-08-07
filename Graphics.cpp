#include "stdafx.h"
#include "Packet.h"
#include "Graphics.h"

using namespace std;

static const QFont DRAW_FONT("Times", Graphics::ObjectWidth - 4, QFont::Bold);

void Graphics::Paint(QPainter& painter, const Ship::SHIPTYPES ship, const Ship::ROTATE rotate) const
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

bool Graphics::AddShip(const Ship::SHIPTYPES ship, const Ship::ROTATE rotate)
{
	if (IsReadyToPlay())
		return false;
	const auto xs = GetMassiveCoords();
	if (!get<0>(xs))
		return false;
	return AddOrRemove(get<1>(xs), get<2>(xs), ship, rotate);
}

void Graphics::RemoveShip()
{
	const auto xs = GetMassiveCoords();
	if (!get<0>(xs))
		return;
	Q_UNUSED(AddOrRemove(get<1>(xs), get<2>(xs), Ship::SHIPTYPES::EMPTY, Ship::ROTATE::NIL));
}

bool Graphics::IsReadyToPlay(const Ship::SHIPTYPES ship) const
{
	if (ship == Ship::SHIPTYPES::EMPTY)
		return Ship::GetMaxShipCount(Ship::SHIPTYPES::CRUISER) == GetShipCount(Ship::SHIPTYPES::CRUISER) &&
		Ship::GetMaxShipCount(Ship::SHIPTYPES::ESMINEC) == GetShipCount(Ship::SHIPTYPES::ESMINEC) &&
		Ship::GetMaxShipCount(Ship::SHIPTYPES::LINKOR) == GetShipCount(Ship::SHIPTYPES::LINKOR) &&
		Ship::GetMaxShipCount(Ship::SHIPTYPES::VEDETTE) == GetShipCount(Ship::SHIPTYPES::VEDETTE);
	return Ship::GetMaxShipCount(ship) == GetShipCount(ship);
}

bool Graphics::IsKilled(unsigned int k, const Ship::BIT bit) const
{
	switch (const Ship& ship = _screenObjects[k]; ship.GetRotate())
	{
	case Ship::ROTATE::STARTRIGHT:
		for (const unsigned int max = k + Ship::GetFloors(ship.GetShipType()); k < max; ++k)
			if (_screenObjects[k].GetBit() != bit)
				return false;
		return true;
	case Ship::ROTATE::STARTDOWN:
		for (const unsigned int max = k + (Ship::GetFloors(ship.GetShipType()) * 10); k < max; k += 10)
			if (_screenObjects[k].GetBit() != bit)
				return false;
		return true;
	default:
		return true;
	}
}

bool Graphics::IsRivalBroken() const
{
	for (unsigned int k = 0; k < _screenObjects.size(); ++k)
		if (_screenObjects[k].GetIsRivalHolding() && !IsKilled(k, Ship::BIT::MYBEAT))
			return false;
	return true;
}

bool Graphics::IsIamBroken() const
{
	for (unsigned int k = 0; k < _screenObjects.size(); ++k)
		if (_screenObjects[k].GetIsMyHolding() && !IsKilled(k, Ship::BIT::RIVALBEAT))
			return false;
	return true;
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

void Graphics::DrawShipRect(QPainter& painter, const Ship::SHIPTYPES ship, const Ship::ROTATE rotate) const
{
	const auto drawShipAndFrame = [&painter](const int x, const int y, const Ship* pShip, const int w = -1, const int h = -1)
	{
		static constexpr int W = BetweenObjects * 2;
		static constexpr int Wh = ObjectWidth + W;
		static const QPen G(Qt::gray, BetweenObjects);
		static const QBrush D(Qt::gray, Qt::Dense6Pattern);
		if (w < 0 || h < 0)
		{
			painter.setPen(G);
			painter.setBrush(D);
			painter.drawRect(x - BetweenObjects, y - BetweenObjects, Wh, Wh);
			return;
		}
		painter.setPen(pShip ? Ship::GetColor(pShip->GetShipType()) : Qt::black);
		painter.setBrush(Qt::NoBrush);
		painter.drawRect(x, y, w, h);
		if (CursorX >= x && CursorX < (x + w) && CursorY >= y && CursorY < (y + h))
		{
			painter.setPen(G);
			painter.setBrush(D);
			painter.drawRect(x - BetweenObjects, y - BetweenObjects, w + W, h + W);
		}
	};

	const auto mBeat = [&painter, &drawShipAndFrame](const int x, const int y, const Ship::BIT bit)
	{
		switch (bit)
		{
		case Ship::BIT::MYBEAT:
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
		case Ship::BIT::RIVALBEAT:
		{
			if (!IsRivalMove)
				return;
			static const QPen Dpen(Qt::red, 3);
			painter.setPen(Dpen);
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

	for (int x = 0, p = 0, xc = Margin; x < 10; ++x, xc += ObjectWidth)
		for (int y = 0, yc = Margin; y < 10; ++y, ++p, yc += ObjectWidth)
		{
			const Ship& s = _screenObjects[p];
			mBeat(xc, yc, s.GetBit());
			const int floors = Ship::GetFloors(s.GetShipType()) * ObjectWidth;
			int tx, ty;
			switch (s.GetRotate())
			{
				int x2, y2;
			case Ship::ROTATE::NIL:
			{
				if (!_shipsAddition)
				{
					drawShipAndFrame(xc, yc, nullptr);
					continue;
				}
				if (ship == Ship::SHIPTYPES::EMPTY || rotate == Ship::ROTATE::NIL)
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
				if (x2 > MaxCoord || y2 > MaxCoord)
					continue;
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
			if (s.GetIsRivalHolding())
				drawShipAndFrame(xc, yc, nullptr);
			if (s.GetIsMyHolding())
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
		return !_screenObjects[static_cast<unsigned int>((y * 10) + x)].GetIsMyHolding();
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

bool Graphics::AddOrRemove(const int startX, const int startY, const Ship::SHIPTYPES ship, const Ship::ROTATE rotate)
{
	if (_screenObjects.size() != 100 || startX < 0 || startX > 9 || startY < 0 || startY > 9)
		throw exception(__func__);

	const auto fSet = [](Ship& obj, const Ship::ROTATE rotate, const Ship::SHIPTYPES sp)
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
		int max = startY + Ship::GetFloors(ship);
		if (max > 10)
			return false;
		if (ship != Ship::SHIPTYPES::EMPTY)
		{
			for (int k = startY; k < max; ++k)
				if (!IsFree(startX, k))
					return false;
		}
		else
			if ((max = Ship::GetFloors(_screenObjects[i].GetShipType())) == 0)
				return false;
		for (int k = startY; k < max; ++k, i += 10u)
			if (ship != Ship::SHIPTYPES::EMPTY)
				fSet(_screenObjects[i], k == startY ? Ship::ROTATE::STARTDOWN : Ship::ROTATE::NIL, ship);
			else
				_screenObjects[i].Delete();
		return true;
	}
	case Ship::ROTATE::STARTRIGHT:
	{
		int max = startX + Ship::GetFloors(ship);
		if (max > 10)
			return false;
		if (ship != Ship::SHIPTYPES::EMPTY)
		{
			for (int k = startX; k < max; ++k)
				if (!IsFree(k, startY))
					return false;
		}
		else
			if ((max = Ship::GetFloors(_screenObjects[i].GetShipType())) == 0)
				return false;
		for (int k = startX; k < max; ++k, ++i)
			if (ship != Ship::SHIPTYPES::EMPTY)
				fSet(_screenObjects[i], k == startX ? Ship::ROTATE::STARTRIGHT : Ship::ROTATE::NIL, ship);
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

int Graphics::GetShipCount(const Ship::SHIPTYPES ship) const
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

void Graphics::SlotShipsAdded(const bool added)
{
	_shipsAddition = !added;
}
