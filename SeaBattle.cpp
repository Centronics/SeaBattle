#include "stdafx.h"
#include "SeaBattle.h"

using namespace std;

SeaBattle::SeaBattle(QWidget *parent) : QMainWindow(parent)
{
	_mainForm.setupUi(this);
	setMouseTracking(true);
	setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint | Qt::MSWindowsFixedSizeDialogHint);
	connect(_mainForm.btnConnect, SIGNAL(clicked()), this, SLOT(BtnConnectClicked()));
	connect(_mainForm.btnServerStart, SIGNAL(clicked()), this, SLOT(BtnServerStartClicked()));
	connect(_mainForm.btnDisconnect, SIGNAL(clicked()), this, SLOT(BtnDisconnect()));
	connect(_mainForm.btnClearShips, SIGNAL(clicked()), this, SLOT(BtnClearShips()));
	_mainForm.btnDisconnect->setEnabled(false);
}

void SeaBattle::BtnClearShips()
{
	_graphics.ClearField();
}

void SeaBattle::BtnConnectClicked()
{
	if (!CheckGameReady())
		return;
	const auto port = GetPort();
	if (!port)
		return;
	_mainForm.btnConnect->setEnabled(false);
	_mainForm.btnServerStart->setEnabled(false);
	_mainForm.btnDisconnect->setEnabled(true);
	_mainForm.lstShipArea->setEnabled(false);
	_mainForm.lstDirection->setEnabled(false);
	if (_clientServer.StartClient(_mainForm.IPAddress->text(), *port))
		Graphics::ShipAddition = false;
	else
		Message("Ошибка при запуске сервера.", "Попробуйте ещё раз.");
}

void SeaBattle::BtnServerStartClicked()
{
	if (!CheckGameReady())
		return;
	const auto port = GetPort();
	if (!port)
		return;
	_mainForm.btnConnect->setEnabled(false);
	_mainForm.btnServerStart->setEnabled(false);
	_mainForm.btnDisconnect->setEnabled(true);
	if (_clientServer.StartServer(*port))
		Graphics::ShipAddition = false;
	else
		Message("Ошибка при запуске сервера.", "Попробуйте ещё раз.");
}

bool SeaBattle::CheckGameReady()
{
	if (_graphics.IsReady())
		return true;
	Message("", "Необходимо расставить все корабли.");
	return false;
}

void SeaBattle::BtnDisconnect()
{
	_mainForm.btnConnect->setEnabled(true);
	_mainForm.btnServerStart->setEnabled(true);
	_clientServer.Disconnect();
	Graphics::ShipAddition = true;
	_graphics.ClearRivalState();
}

void SeaBattle::paintEvent(QPaintEvent *event)
{
	Q_UNUSED(event);
	QPainter painter(this);
	const auto ship = GetSelectedShip();
	if (get<0>(ship))
		_graphics.Paint(painter, get<1>(ship), get<2>(ship));
	else
		_graphics.Paint(painter, Ship::SHIPS::EMPTY, Ship::STATE::NIL);
}

tuple<bool, Ship::SHIPS, Ship::STATE, QListWidgetItem*> SeaBattle::GetSelectedShip() const
{
	if (_graphics.IsReady())
		return make_tuple(false, Ship::SHIPS::EMPTY, Ship::STATE::NIL, nullptr);
	Ship::SHIPS ship;
	Ship::STATE state;
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
		return make_tuple(false, Ship::SHIPS::EMPTY, Ship::STATE::NIL, nullptr);
	}
	switch (_mainForm.lstDirection->currentRow())
	{
	case 0:
		state = Ship::STATE::STARTRIGHT;
		break;
	case 1:
		state = Ship::STATE::STARTDOWN;
		break;
	default:
		return make_tuple(false, Ship::SHIPS::EMPTY, Ship::STATE::NIL, nullptr);
	}
	return make_tuple(true, ship, state, _mainForm.lstShipArea->item(_mainForm.lstShipArea->currentRow()));
}

void SeaBattle::AddShip()
{
	const auto selShip = GetSelectedShip();
	if (!get<0>(selShip))
		return;
	if (Ship::GetMaxShipCount(get<1>(selShip)) == _graphics.GetShipCount(get<1>(selShip)))
	{
		Message("Таких кораблей поставлено достаточно!", "Невозможно поставить корабль!");
		return;
	}
	if (!_graphics.AddShip(get<1>(selShip), get<2>(selShip)))
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
	const int shipCount = _graphics.GetShipCount(get<1>(selShip));
	QString str = get<3>(selShip)->text();
	str[39] = QString::number(shipCount)[0];
	get<3>(selShip)->setText(str);
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
	if (_clientServer.GetGameState() == ClientServer::DOIT::STARTGAME)
	{
		AddShip();
		repaint();
		return;
	}
	_packet.WriteData(ClientServer::DOIT::HIT, *coord);
	_clientServer.Send(_packet);
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
		if (_clientServer.GetGameState() != ClientServer::DOIT::STARTGAME)
			return;
		_graphics.RemoveShip();
		RenewShipCount();
		repaint();
		return;
	default:
		return;
	}
}

optional<int> SeaBattle::GetPort()
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
