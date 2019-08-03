#include "stdafx.h"
#include "SeaBattle.h"
#include "Server.h"
#include "Client.h"

using namespace std;

SeaBattle::SeaBattle(QWidget *parent) : QWidget(parent), _graphics(this)
{
	_mainForm.setupUi(this);
	setMouseTracking(true);
	setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint | Qt::MSWindowsFixedSizeDialogHint);
	connect(_mainForm.btnConnect, SIGNAL(clicked()), this, SLOT(SlotBtnConnectClicked()));
	connect(_mainForm.btnServerStart, SIGNAL(clicked()), this, SLOT(SlotBtnServerStartClicked()));
	connect(_mainForm.btnDisconnect, SIGNAL(clicked()), this, SLOT(SlotBtnDisconnect()));
	connect(_mainForm.btnClearShips, SIGNAL(clicked()), this, SLOT(SlotBtnClearShips()));
	connect(this, SIGNAL(SignalShipsAdded(bool)), &_graphics, SLOT(SlotShipsAdded(bool)));
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
	_clientServer.reset();
	_graphics.ClearRivalState();
}

void SeaBattle::SlotConnected(const bool isOK, const QString& objName, const QString& message)
{
	if (isOK)
		emit SignalShipsAdded(true);
	else
		Message(objName, message);
}

void SeaBattle::paintEvent(QPaintEvent *event)
{
	Q_UNUSED(event);
	QPainter painter(this);
	const auto ship = GetSelectedShip();
	if (get<0>(ship))
		_graphics.Paint(painter, get<0>(ship), get<1>(ship));
	else
		_graphics.Paint(painter);
}

tuple<optional<Ship::SHIPS>, Ship::ROTATE, QListWidgetItem*> SeaBattle::GetSelectedShip() const
{
	if (_graphics.IsReadyToPlay())
		return make_tuple(nullopt, Ship::ROTATE::NIL, nullptr);
	Ship::SHIPS ship;
	Ship::ROTATE rotate;
	switch (_mainForm.lstShipArea->currentRow())
	{
	case 0:
		ship = Ship::SHIPS::LINKOR;
		break;
	case 1:
		ship = Ship::SHIPS::CRUISER;
		break;
	case 2:
		ship = Ship::SHIPS::ESMINEC;
		break;
	case 3:
		ship = Ship::SHIPS::VEDETTE;
		break;
	default:
		return make_tuple(nullopt, Ship::ROTATE::NIL, nullptr);
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
		return make_tuple(nullopt, Ship::ROTATE::NIL, nullptr);
	}
	return make_tuple(ship, rotate, _mainForm.lstShipArea->item(_mainForm.lstShipArea->currentRow()));
}

void SeaBattle::AddShip()
{
	const auto selShip = GetSelectedShip();
	if (!get<0>(selShip))
		return;
	if (Ship::GetMaxShipCount(*get<0>(selShip)) == _graphics.GetShipCount(*get<0>(selShip)))
	{
		Message("Таких кораблей поставлено достаточно!", "Невозможно поставить корабль!");
		return;
	}
	if (!_graphics.AddShip(*get<0>(selShip), get<1>(selShip)))
	{
		Message("Сюда нельзя поставить корабль.", "Переставьте в другое место.");
		return;
	}
	RenewShipCount();
}

void SeaBattle::RenewShipCount() const
{
	const auto selShip = GetSelectedShip();
	if (!get<0>(selShip))
		return;
	const int shipCount = _graphics.GetShipCount(*get<0>(selShip));
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
	if (_graphics.IsShipsAddition())
	{
		AddShip();
		repaint();
		return;
	}
	_clientServer->SendHit(*coord);
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
		if (!_graphics.IsShipsAddition())
			return;
		_graphics.RemoveShip();
		RenewShipCount();
		repaint();
		return;
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

void SeaBattle::Message(const QString& m1, const QString& infoMessage)
{
	QMessageBox msgBox(this);
	if (!m1.isEmpty())
		msgBox.setText(m1);
	if (!infoMessage.isEmpty())
		msgBox.setInformativeText(infoMessage);
	msgBox.setStandardButtons(QMessageBox::Ok);
	msgBox.setDefaultButton(QMessageBox::Ok);
	msgBox.exec();
}