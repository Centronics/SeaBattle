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
	connect(_mainForm.btnClearShips, SIGNAL(clicked()), this, SLOT(SlotBtnClearShipsClicked()));
	connect(_mainForm.btnConnect, SIGNAL(clicked()), this, SLOT(SlotBtnConnectClicked()));
	connect(_mainForm.btnServerStart, SIGNAL(clicked()), this, SLOT(SlotBtnServerStartClicked()));
	connect(_mainForm.btnDisconnect, SIGNAL(clicked()), this, SLOT(SlotBtnDisconnectClicked()));
	_mainForm.btnDisconnect->setEnabled(false);
	_mainForm.lstShipArea->setCurrentRow(0);
	_mainForm.lstDirection->setCurrentRow(0);
}

void SeaBattle::SlotBtnClearShipsClicked()
{
	_graphics.ClearField();
	RenewShipCount();
	repaint();
}

void SeaBattle::SlotBtnConnectClicked()
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
	Initialize<Client>()->Connect(_mainForm.txtIPAddress->text(), *port);
	Graphics::ShipAddition = false;
	Graphics::IsRivalMove = false;
}

void SeaBattle::SlotBtnServerStartClicked()
{
	if (!CheckGameReady())
		return;
	const auto port = GetPort();
	if (!port)
	{
		Message("Порт не указан.", "Укажите порт, который необходимо прослушивать!");
		return;
	}
	_mainForm.btnConnect->setEnabled(false);
	_mainForm.btnServerStart->setEnabled(false);
	_mainForm.btnDisconnect->setEnabled(true);
	_mainForm.btnClearShips->setEnabled(false);
	_mainForm.txtIPAddress->setReadOnly(true);
	_mainForm.txtPort->setReadOnly(true);
	_mainForm.lstShipArea->setEnabled(false);
	_mainForm.lstDirection->setEnabled(false);
	Initialize<Server>()->Listen(*port);
	Graphics::ShipAddition = false;
	Graphics::IsRivalMove = true;
	repaint();
}

bool SeaBattle::CheckGameReady()
{
	if (_graphics.IsReadyToPlay())
		return true;
	Message("", "Необходимо расставить все корабли.");
	return false;
}

void SeaBattle::SlotBtnDisconnectClicked()
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
			return;
		Impact(true);
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
	Impact(false);
	repaint();
}

void SeaBattle::paintEvent(QPaintEvent* event)
{
	Q_UNUSED(event);
	QPainter painter(this);
	const auto selShip = GetSelectedShip();
	if (get<0>(selShip) != Ship::TYPES::EMPTY)
		_graphics.Paint(painter, get<0>(selShip), get<1>(selShip));
	else
		_graphics.Paint(painter);
}

void SeaBattle::leaveEvent(QEvent* event)
{
	Graphics::CursorX = -1;
	Graphics::CursorY = -1;
	repaint();
}

tuple<Ship::TYPES, Ship::ROTATE, QListWidgetItem*> SeaBattle::GetSelectedShip() const
{
	if (_graphics.IsReadyToPlay())
		return make_tuple(Ship::TYPES::EMPTY, Ship::ROTATE::NIL, nullptr);
	Ship::TYPES ship;
	Ship::ROTATE rotate;
	switch (_mainForm.lstShipArea->currentRow())
	{
	case 0:
		ship = Ship::TYPES::LINKOR;
		break;
	case 1:
		ship = Ship::TYPES::CRUISER;
		break;
	case 2:
		ship = Ship::TYPES::ESMINEC;
		break;
	case 3:
		ship = Ship::TYPES::VEDETTE;
		break;
	default:
		return make_tuple(Ship::TYPES::EMPTY, Ship::ROTATE::NIL, nullptr);
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
		return make_tuple(Ship::TYPES::EMPTY, Ship::ROTATE::NIL, nullptr);
	}
	return make_tuple(ship, rotate, _mainForm.lstShipArea->item(_mainForm.lstShipArea->currentRow()));
}

void SeaBattle::AddShip()
{
	const auto selShip = GetSelectedShip();
	if (get<0>(selShip) == Ship::TYPES::EMPTY)
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
	const auto f = [this](const Ship::TYPES shipType, QListWidgetItem* const item)
	{
		QString str = item->text();
		int n;
		switch (shipType)
		{
		case Ship::TYPES::LINKOR:
			n = 41;
			break;
		case Ship::TYPES::CRUISER:
			n = 45;
			break;
		case Ship::TYPES::ESMINEC:
			n = 44;
			break;
		case Ship::TYPES::VEDETTE:
			n = 35;
			break;
		default:
			throw exception("RenewShipCount");
		}
		str[n] = QString::number(_graphics.GetShipCount(shipType))[0];
		item->setText(str);
	};

	f(Ship::TYPES::LINKOR, _mainForm.lstShipArea->item(0));
	f(Ship::TYPES::CRUISER, _mainForm.lstShipArea->item(1));
	f(Ship::TYPES::ESMINEC, _mainForm.lstShipArea->item(2));
	f(Ship::TYPES::VEDETTE, _mainForm.lstShipArea->item(3));
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
		Impact(false);
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
			Message("Корабль отсутствует.", "В указанном месте нет корабля.");
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
	const int port = _mainForm.txtPort->text().toInt(&ok);
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

void SeaBattle::Impact(const bool disconnect)
{
	const auto pStop = [this]
	{
		Graphics::ShipAddition = true;
		Graphics::IsRivalMove = false;
		_clientServer->Close();
		_clientServer.reset();
	};

	switch (_graphics.IsBroken())
	{
	case Graphics::BROKEN::ME:
		pStop();
		Message("Поражение!", "Вы проиграли.");
		return;
	case Graphics::BROKEN::RIVAL:
		pStop();
		Message("Победа!", "Соперник потерпел поражение.");
		return;
	case Graphics::BROKEN::NOTHING:
		if (disconnect)
			pStop();
		Message("Игра прекращена.", "Соединение разорвано.");
		return;
	default:
		throw exception(__func__);
	}
}
