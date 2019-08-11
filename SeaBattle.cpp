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
}

void SeaBattle::SlotBtnClearShipsClicked()
{
	_graphics.ClearField();
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
	Initialize<Client>()->Connect(_mainForm.IPAddress->text(), *port);
	Graphics::ShipAddition = false;
	Graphics::IsRivalMove = false;
}

void SeaBattle::SlotBtnServerStartClicked()
{
	if (!CheckGameReady())
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

bool SeaBattle::CheckGameReady()
{
	if (_graphics.IsReadyToPlay())
		return true;
	Message("", "���������� ���������� ��� �������.");
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
		Message("������.", packet.ErrorString());
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
		throw exception("������������ �����.");
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
		Message("����� �������� ���������� ����������.", "���������� ��������� �������.");
		return;
	case Graphics::SHIPADDITION::NOCOORD:
		Message("���� ������ ��������� �������.", "����������� � ������ �����.");
		return;
	case Graphics::SHIPADDITION::NOTFREE:
		Message("����� ������.", "������ ������� ����� ������� ������.");
		return;
	case Graphics::SHIPADDITION::INCORRECTMODE:
		Message("�������� �����.", "������� ��������� ��� ������� ����� ������ �� ������ ����.");
		return;
	default:
		throw exception(__func__);
	}
}

void SeaBattle::RenewShipCount() const
{
	const auto selShip = GetSelectedShip();
	if (get<0>(selShip) == Ship::TYPES::EMPTY)
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
			Message("���� ������ ��������� �������.", "����������� � ������ �����.");
			return;
		case Graphics::SHIPADDITION::NOSHIP:
			Message("������� �����������.", "� ��������� ����� ��� �������.");
			return;
		case Graphics::SHIPADDITION::INCORRECTMODE:
			Message("�������� �����.", "��������� ��� ������� ������� ����� ������ �� ������ ����.");
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
	Message("������ ��� �����������.", "���� ������ ���� ������.");
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
		Message("���������!", "�� ���������.");
		return;
	case Graphics::BROKEN::RIVAL:
		pStop();
		Message("������!", "�������� �������� ���������.");
		return;
	case Graphics::BROKEN::NOTHING:
		if (disconnect)
			pStop();
		Message("���� ����������.", "���������� ���������.");
		return;
	default:
		throw exception(__func__);
	}
}
