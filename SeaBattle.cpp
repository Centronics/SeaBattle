#include "stdafx.h"
#include "SeaBattle.h"
#include "Server.h"
#include "Client.h"

using namespace std;

SeaBattle::SeaBattle(QWidget* parent) : QWidget(parent), _graphics(this)
{
	_mainForm.setupUi(this);
	setMouseTracking(true);
	setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint | Qt::MSWindowsFixedSizeDialogHint);
	connect(_mainForm.btnConnect, SIGNAL(clicked()), this, SLOT(SlotBtnConnectClicked()));
	connect(_mainForm.btnServerStart, SIGNAL(clicked()), this, SLOT(SlotBtnServerStartClicked()));
	connect(_mainForm.btnDisconnect, SIGNAL(clicked()), this, SLOT(SlotBtnDisconnect()));
	connect(_mainForm.btnClearShips, SIGNAL(clicked()), this, SLOT(SlotBtnClearShips()));
	_mainForm.btnDisconnect->setEnabled(false);
}

void SeaBattle::SlotBtnClearShips()
{
	_graphics.ClearField();
}

void SeaBattle::SlotBtnConnectClicked()
{
	if (!SlotCheckGameReady())
		return;
	const auto port = GetPort();
	if (!port)
		return;
	_mainForm.btnConnect->setEnabled(false);
	_mainForm.btnServerStart->setEnabled(false);
	_mainForm.btnDisconnect->setEnabled(true);
	_mainForm.lstShipArea->setEnabled(false);
	_mainForm.lstDirection->setEnabled(false);
	Initialize<Client>()->Connect(_mainForm.IPAddress->text(), *port);
	Graphics::ShipAddition = false;
	Graphics::IsRivalMove = false;
}

void SeaBattle::SlotBtnServerStartClicked()
{
	if (!SlotCheckGameReady())
		return;
	const auto port = GetPort();
	if (!port)
		return;
	_mainForm.btnConnect->setEnabled(false);
	_mainForm.btnServerStart->setEnabled(false);
	_mainForm.btnDisconnect->setEnabled(true);
	Initialize<Server>()->Listen(*port);
	Graphics::ShipAddition = false;
	Graphics::IsRivalMove = true;
}

bool SeaBattle::SlotCheckGameReady()
{
	if (_graphics.IsReadyToPlay())
		return true;
	Message("", "Необходимо расставить все корабли.");
	return false;
}

void SeaBattle::SlotBtnDisconnect()
{
	_mainForm.btnConnect->setEnabled(true);
	_mainForm.btnServerStart->setEnabled(true);
	_mainForm.btnClearShips->setEnabled(true);
	_mainForm.btnDisconnect->setEnabled(false);
	_clientServer.reset();
	_graphics.ClearRivalState();
	Graphics::ShipAddition = true;
	Graphics::IsRivalMove = false;
	repaint();
}

void SeaBattle::SlotReceive(const Packet& packet)
{
	if (!packet)
	{
		Message("Ошибка.", packet.ErrorString());
		Graphics::ShipAddition = true;
		Graphics::IsRivalMove = false;
		repaint();
		return;
	}
	DOIT doit;
	if (packet.ReadData(doit))
	{
		if (doit != DOIT::STOPGAME)
			throw exception("Некорректный пакет.");
		Message("Игра прекращена.", "Соперник прекратил игру.");
		Graphics::ShipAddition = true;
		Graphics::IsRivalMove = false;
		repaint();
		return;
	}
	quint8 param;
	if (!packet.ReadData(doit, param))
		return;
	if (doit != DOIT::HIT)
		throw exception("Некорректный пакет.");
	if (!Graphics::IsRivalMove)
		return;
	_graphics.RivalHit(param);
	Graphics::IsRivalMove = false;
	Impact();
	repaint();
}

void SeaBattle::paintEvent(QPaintEvent* event)
{
	Q_UNUSED(event);
	QPainter painter(this);
	const auto ship = GetSelectedShip();
	if (get<0>(ship) != Ship::SHIPTYPES::EMPTY)
		_graphics.Paint(painter, get<0>(ship), get<1>(ship));
	else
		_graphics.Paint(painter);
}

tuple<Ship::SHIPTYPES, Ship::ROTATE, QListWidgetItem*> SeaBattle::GetSelectedShip() const
{
	if (_graphics.IsReadyToPlay())
		return make_tuple(Ship::SHIPTYPES::EMPTY, Ship::ROTATE::NIL, nullptr);
	Ship::SHIPTYPES ship;
	Ship::ROTATE rotate;
	switch (_mainForm.lstShipArea->currentRow())
	{
	case 0:
		ship = Ship::SHIPTYPES::LINKOR;
		break;
	case 1:
		ship = Ship::SHIPTYPES::CRUISER;
		break;
	case 2:
		ship = Ship::SHIPTYPES::ESMINEC;
		break;
	case 3:
		ship = Ship::SHIPTYPES::VEDETTE;
		break;
	default:
		return make_tuple(Ship::SHIPTYPES::EMPTY, Ship::ROTATE::NIL, nullptr);
	}
	switch (_mainForm.lstDirection->currentRow())
	{
	case 0:
		rotate = Ship::ROTATE::STARTRIGHT;
		break;
	case 1:
		rotate = Ship::ROTATE::STARTDOWN;
		break;
	default:
		return make_tuple(Ship::SHIPTYPES::EMPTY, Ship::ROTATE::NIL, nullptr);
	}
	return make_tuple(ship, rotate, _mainForm.lstShipArea->item(_mainForm.lstShipArea->currentRow()));
}

void SeaBattle::AddShip()
{
	const auto selShip = GetSelectedShip();
	if (get<0>(selShip) == Ship::SHIPTYPES::EMPTY)
		return;
	switch (_graphics.AddShip(get<0>(selShip), get<1>(selShip)))
	{
	case Graphics::SHIPADDITION::OK:
		RenewShipCount();
		return;
	case Graphics::SHIPADDITION::MANY:
		Message("Таких кораблей поставлено достаточно.", "Невозможно поставить корабль.");
		return;
	case Graphics::SHIPADDITION::NOCOORD:
		Message("Сюда нельзя поставить корабль.", "Переставьте в другое место.");
		return;
	case Graphics::SHIPADDITION::NOTFREE:
		Message("Место занято.", "Другие корабли стоят слишком близко.");
		return;
	case Graphics::SHIPADDITION::INCORRECTMODE:
		Message("Неверный режим.", "Корабли добавлять или удалять можно только до начала игры.");
		return;
	default:
		throw exception(__func__);
	}
}

void SeaBattle::RenewShipCount() const
{
	const auto selShip = GetSelectedShip();
	if (get<0>(selShip) == Ship::SHIPTYPES::EMPTY)
		return;
	const int shipCount = _graphics.GetShipCount(get<0>(selShip));
	QString str = get<2>(selShip)->text();
	str[39] = QString::number(shipCount)[0];
	get<2>(selShip)->setText(str);
}

void SeaBattle::mouseMoveEvent(QMouseEvent* event)
{
	Graphics::CursorX = event->x();
	Graphics::CursorY = event->y();
	repaint();
}

void SeaBattle::mouseReleaseEvent(QMouseEvent* event)
{
	Graphics::Clicked = event->button() == Qt::LeftButton;
	const optional<quint8> coord = _graphics.GetCoord();
	if (!Graphics::Clicked || !coord)
		return;
	if (Graphics::ShipAddition)
		AddShip();
	else
	{
		if (Graphics::IsRivalMove)
			return;
		Graphics::IsRivalMove = true;
		_graphics.MyHit(*coord);
		_clientServer->SendHit(*coord);
		Impact();
	}
	repaint();
}

void SeaBattle::keyReleaseEvent(QKeyEvent* event)
{
	switch (event->key())
	{
	case Qt::Key::Key_Escape:
		QApplication::quit();
		return;
	case Qt::Key::Key_Delete:
		switch (_graphics.RemoveShip())
		{
		case Graphics::SHIPADDITION::OK:
			RenewShipCount();
			repaint();
			return;
		case Graphics::SHIPADDITION::NOCOORD:
			Message("Сюда нельзя поставить корабль.", "Переставьте в другое место.");
			return;
		case Graphics::SHIPADDITION::NOSHIP:
			Message("", "В указанном месте корабля нет.");
			return;
		case Graphics::SHIPADDITION::INCORRECTMODE:
			Message("Неверный режим.", "Добавлять или удалять корабли можно только до начала игры.");
			return;
		default:
			throw exception(__func__);
		}
	default:
		return;
	}
}

optional<quint16> SeaBattle::GetPort()
{
	bool ok;
	const int port = _mainForm.Port->text().toInt(&ok);
	if (ok)
		return port;
	Message("Ошибка при подключении.", "Порт должен быть числом.");
	return nullopt;
}

void SeaBattle::Message(const QString& comment, const QString& infoMessage)
{
	QMessageBox msgBox(this);
	if (!comment.isEmpty())
		msgBox.setText(comment);
	if (!infoMessage.isEmpty())
		msgBox.setInformativeText(infoMessage);
	msgBox.setStandardButtons(QMessageBox::Ok);
	msgBox.setDefaultButton(QMessageBox::Ok);
	msgBox.exec();
}

void SeaBattle::Impact()
{
	const auto pStop = [this]
	{
		Graphics::ShipAddition = true;
		Graphics::IsRivalMove = false;
		_clientServer->Close();
		_clientServer.reset();
	};

	if (_graphics.IsRivalBroken())
		pStop();
	else
		if (_graphics.IsIamBroken())
			pStop();
}
