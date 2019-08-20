#include "stdafx.h"
#include "SeaBattle.h"
#include "Server.h"
#include "Client.h"

using namespace std;

SeaBattle::SeaBattle(QWidget* parent) : QWidget(parent), _graphics(this)
{
	_mainForm.setupUi(this);
	_mainForm.frmDrawing->installEventFilter(this);
	setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint | Qt::MSWindowsFixedSizeDialogHint);
	connect(_mainForm.btnClearShips, SIGNAL(clicked()), SLOT(SlotBtnClearShipsClicked()));
	connect(_mainForm.btnConnect, SIGNAL(clicked()), SLOT(SlotBtnConnectClicked()));
	connect(_mainForm.btnServerStart, SIGNAL(clicked()), SLOT(SlotBtnServerStartClicked()));
	connect(_mainForm.btnDisconnect, SIGNAL(clicked()), SLOT(SlotBtnDisconnectClicked()));
	_mainForm.lstShipArea->setCurrentRow(0);
	_mainForm.lstDirection->setCurrentRow(0);
}

void SeaBattle::SlotBtnClearShipsClicked()
{
	_graphics.ClearField();
	RenewShipCount();
	update();
}

void SeaBattle::SlotBtnConnectClicked()
{
	if (!CheckGameReady())
		return;
	const optional<quint16> port = GetPort();
	if (!port)
	{
		Message("���� �� ������.", "������� ����, � �������� ���������� ������������!");
		return;
	}
	OffButtons();
	Initialize<Client>()->Connect(_mainForm.txtIPAddress->text(), *port);
	Graphics::ShipAddition = false;
	Graphics::IsRivalMove = false;
	update();
}

void SeaBattle::SlotBtnServerStartClicked()
{
	if (!CheckGameReady())
		return;
	const optional<quint16> port = GetPort();
	if (!port)
	{
		Message("���� �� ������.", "������� ����, ������� ���������� ������������!");
		return;
	}
	OffButtons();
	Initialize<Server>()->Listen(*port);
	Graphics::ShipAddition = false;
	Graphics::IsRivalMove = true;
	update();
}

bool SeaBattle::eventFilter(QObject* watched, QEvent* event)
{
	if (watched != _mainForm.frmDrawing)
		return QObject::eventFilter(watched, event);
	// ReSharper disable once CppDefaultCaseNotHandledInSwitchStatement
	switch (event->type())
	{
	case QEvent::Leave:
		Graphics::CursorX = -1;
		Graphics::CursorY = -1;
		_mainForm.frmDrawing->update();
		break;
	case QEvent::Paint:
	{
		QPainter painter(_mainForm.frmDrawing);
		const auto selShip = GetSelectedShip();
		_graphics.Paint(painter, get<0>(selShip), get<1>(selShip));
		break;
	}
	}
	return QObject::eventFilter(watched, event);
}

void SeaBattle::OffButtons(const bool off) const
{
	const bool t = !off;
	_mainForm.btnConnect->setEnabled(t);
	_mainForm.btnServerStart->setEnabled(t);
	_mainForm.btnClearShips->setEnabled(t);
	_mainForm.lstShipArea->setEnabled(t);
	_mainForm.lstDirection->setEnabled(t);
	_mainForm.btnDisconnect->setEnabled(off);
	_mainForm.txtPort->setReadOnly(off);
	_mainForm.txtIPAddress->setReadOnly(off);
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
	OffButtons(false);
	_clientServer.reset();
	_graphics.ClearRivalState();
	Graphics::ShipAddition = true;
	Graphics::IsRivalMove = false;
	update();
}

void SeaBattle::SlotReceive(const Packet& packet)
{
	if (!packet)
	{
		Message("������.", packet.ErrorString());
		Graphics::ShipAddition = true;
		Graphics::IsRivalMove = false;
		update();
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
	update();
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

void SeaBattle::RemoveShip()
{
	switch (_graphics.RemoveShip())
	{
	case Graphics::SHIPADDITION::OK:
		RenewShipCount();
		update();
		return;
	case Graphics::SHIPADDITION::NOCOORD:
		Message("���� ������ ��������� �������.", "����������� � ������ �����.");
		return;
	case Graphics::SHIPADDITION::INCORRECTMODE:
		Message("�������� �����.", "��������� ��� ������� ������� ����� ������ �� ������ ����.");
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
	update();
}

void SeaBattle::mouseReleaseEvent(QMouseEvent* event)
{
	switch (event->button())
	{
	case Qt::LeftButton:
		Graphics::Clicked = true;
		if (!Graphics::ShipAddition)
		{
			if (Graphics::IsRivalMove)
			{
				Graphics::Clicked = false;
				return;
			}
			const optional<quint8> coord = _graphics.GetCoord();
			if (!coord)
			{
				Graphics::Clicked = false;
				return;
			}
			_graphics.MyHit(*coord);
			_clientServer->SendHit(*coord);
			Impact(false);
			Graphics::IsRivalMove = true;
			break;
		}
		AddShip();
		break;
	case Qt::RightButton:
		RemoveShip();
		break;
	default:
		return;
	}
	update();
}

void SeaBattle::keyReleaseEvent(QKeyEvent* event)
{
	switch (event->key())
	{
	case Qt::Key::Key_Escape:
		QApplication::quit();
		return;
	case Qt::Key::Key_Delete:
		RemoveShip();
		return;
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
