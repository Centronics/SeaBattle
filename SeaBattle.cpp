#include "stdafx.h"
#include "SeaBattle.h"
#include "Packet.h"
#include "Client.h"
#include "Server.h"
#include "MyMessageBox.h"

using namespace std;

SeaBattle::SeaBattle(QWidget* parent) noexcept : QWidget(parent)
{
	_mainForm.setupUi(this);
	setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint | Qt::MSWindowsFixedSizeDialogHint);
	_helpForm.setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint | Qt::MSWindowsFixedSizeDialogHint);
	_mainForm.frmDrawing->installEventFilter(this);
	_mainForm.txtIPAddress->installEventFilter(this);
	_mainForm.txtPort->installEventFilter(this);
	_mainForm.btnConnect->installEventFilter(this);
	_mainForm.btnServerStart->installEventFilter(this);
	_mainForm.btnDisconnect->installEventFilter(this);
	_mainForm.btnHelp->installEventFilter(this);
	_mainForm.lstShipArea->installEventFilter(this);
	_mainForm.lstDirection->installEventFilter(this);
	_mainForm.lstShipArea->setCurrentRow(0);
	_mainForm.lstDirection->setCurrentRow(0);
	LoadParameters();
	Q_UNUSED(connect(_mainForm.btnHelp, SIGNAL(clicked()), SLOT(SlotBtnHelpClicked())));
	Q_UNUSED(connect(_mainForm.btnConnect, SIGNAL(clicked()), SLOT(SlotBtnConnectClicked())));
	Q_UNUSED(connect(_mainForm.btnServerStart, SIGNAL(clicked()), SLOT(SlotBtnServerStartClicked())));
	Q_UNUSED(connect(_mainForm.btnDisconnect, SIGNAL(clicked()), SLOT(SlotBtnDisconnectClicked())));
	Q_UNUSED(connect(_mainForm.lstShipArea, SIGNAL(currentRowChanged(int)), SLOT(SlotLstChange(int))));
	Q_UNUSED(connect(_mainForm.lstDirection, SIGNAL(currentRowChanged(int)), SLOT(SlotLstChange(int))));
	Q_UNUSED(connect(this, SIGNAL(SigMessage(QString, QString, qint32, bool)), SLOT(SlotMessage(QString, QString, qint32, bool)), Qt::QueuedConnection));
	Q_UNUSED(connect(this, SIGNAL(SigGrab()), SLOT(SlotGrab()), Qt::QueuedConnection));
}

void SeaBattle::SlotLstChange(int)
{
	update();
}

void SeaBattle::SlotBtnHelpClicked()
{
	_helpForm.show();
}

void SeaBattle::SlotBtnConnectClicked()
{
	SaveParameters();
	if (!CheckGameReady())
		return;
	const optional<quint16> port = GetPort();
	if (!port)
	{
		Message("Укажите допустимое значение прослушиваемого порта.", "Укажите порт, к которому необходимо подключиться!");
		return;
	}
	OffButtons();
	Graphics::ConnectionStatus = Graphics::CONNECTIONSTATUS::CLIENT;
	Graphics::IsRivalMove = false;
	_graphics.ClearBitShips();
	Initialize<Client>()->Connect(_mainForm.txtIPAddress->text(), *port);
	update();
}

void SeaBattle::SlotBtnServerStartClicked()
{
	SaveParameters();
	if (!CheckGameReady())
		return;
	const optional<quint16> port = GetPort();
	if (!port)
	{
		Message("Укажите допустимое значение прослушиваемого порта.", "Укажите порт, который необходимо прослушивать!");
		return;
	}
	OffButtons();
	Graphics::ConnectionStatus = Graphics::CONNECTIONSTATUS::SERVER;
	Graphics::IsRivalMove = true;
	_graphics.ClearBitShips();
	Initialize<Server>()->Listen(*port);
	update();
}

void SeaBattle::SaveParameters() const
{
	QDomDocument doc(QString{});

	const auto createElement = [&doc](const QString& tagName, const QString& value)
	{
		QDomElement domElement = doc.createElement(tagName);
		doc.appendChild(domElement);
		domElement.appendChild(doc.createTextNode(value.trimmed()));
		return domElement;
	};

	QDomElement domElement = doc.createElement("SeaBattle");
	doc.appendChild(domElement);
	domElement.appendChild(createElement("IPAddress", _mainForm.txtIPAddress->text()));
	domElement.appendChild(createElement("Port", _mainForm.txtPort->text()));
	QFile file(SettingsFileName);
	if (file.open(QIODevice::WriteOnly | QIODevice::Truncate))
		QTextStream(&file) << doc.toString();
}

void SeaBattle::LoadParameters() const
{
	QFile file(SettingsFileName);
	if (!file.open(QIODevice::ReadOnly))
		return;

	QDomDocument domDoc;
	if (!domDoc.setContent(&file))
		return;
	QDomNode node = domDoc.documentElement().firstChild();
	if (node.isNull() || !node.isElement())
		return;
	QDomElement el = node.toElement();
	if (el.isNull() || el.tagName() != "IPAddress")
		return;
	const QString strIP = el.text();
	node = node.nextSibling();
	if (node.isNull())
		return;
	el = node.toElement();
	if (el.isNull() || el.tagName() != "Port")
		return;
	_mainForm.txtIPAddress->setText(strIP);
	_mainForm.txtPort->setText(el.text());
}

bool SeaBattle::eventFilter(QObject* watched, QEvent* event)
{
	switch (event->type())
	{
	case QEvent::Leave:
		if (watched != _mainForm.frmDrawing)
			return QWidget::eventFilter(watched, event);
		Graphics::CursorX = -1;
		Graphics::CursorY = -1;
		_mainForm.frmDrawing->update();
		return true;
	case QEvent::Paint:
	{
		if (watched != _mainForm.frmDrawing)
			return QWidget::eventFilter(watched, event);
		QPainter painter(_mainForm.frmDrawing);
		const auto selShip = GetSelectedShip();
		_graphics.Paint(painter, get<0>(selShip), get<1>(selShip), _neutralColor);
		return true;
	}
	case QEvent::KeyPress:
	{
		const bool isNotButton = watched != _mainForm.btnConnect && watched != _mainForm.btnServerStart && watched != _mainForm.btnDisconnect && watched != _mainForm.btnHelp;
		if (watched != _mainForm.txtIPAddress && watched != _mainForm.txtPort && isNotButton && watched != _mainForm.lstShipArea && watched != _mainForm.lstDirection)
			return QWidget::eventFilter(watched, event);
		switch (const auto e = reinterpret_cast<QKeyEvent*>(event); e->key())
		{
		case Qt::Key::Key_Space:
			if (Graphics::ConnectionStatus != Graphics::CONNECTIONSTATUS::DISCONNECTED || e->isAutoRepeat())
				return true;
			if (_mainForm.lstDirection->currentRow() == 0)
				_mainForm.lstDirection->setCurrentRow(1);
			else
				_mainForm.lstDirection->setCurrentRow(0);
			return true;
		case Qt::Key::Key_Enter:
		case Qt::Key_Return:
			if (isNotButton)
				return QWidget::eventFilter(watched, event);
			if (e->isAutoRepeat())
				return true;
			if (watched == _mainForm.btnConnect)
				SlotBtnConnectClicked();
			else
				if (watched == _mainForm.btnServerStart)
					SlotBtnServerStartClicked();
				else
					if (watched == _mainForm.btnDisconnect)
						SlotBtnDisconnectClicked();
					else
						if (watched == _mainForm.btnHelp)
							SlotBtnHelpClicked();
			return true;
		default:
			return QWidget::eventFilter(watched, event);
		}
	}
	default:
		return QWidget::eventFilter(watched, event);
	}
}

void SeaBattle::paintEvent(QPaintEvent*)
{
	QPainter painter(this);
	Graphics::DrawMoveQuad(painter);
	if (static bool bNeed = true; bNeed)
	{
		emit SigGrab();
		bNeed = false;
	}
}

void SeaBattle::OffButtons(const bool off) const
{
	const bool t = !off;
	_mainForm.btnConnect->setEnabled(t);
	_mainForm.btnServerStart->setEnabled(t);
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
	Message(QString{}, "Необходимо расставить все корабли.");
	return false;
}

void SeaBattle::SlotBtnDisconnectClicked()
{
	ExitGame(true);
	_clientServer->Close();
	update();
}

void SeaBattle::ExitGame(const bool clearBit)
{
	OffButtons(false);
	Graphics::ConnectionStatus = Graphics::CONNECTIONSTATUS::DISCONNECTED;
	Graphics::IsRivalMove = false;
	if (clearBit)
		_graphics.ClearBitShips();
	else
		_graphics.DrawRivals();
}

void SeaBattle::SlotReceive(const Packet packet, NetworkInterface::STATUS* status)  // NOLINT(performance-unnecessary-value-param)
{
	NetworkInterface::STATUS s;
	switch (QString errStr; packet.GetState(&errStr))
	{
	case Packet::STATE::NOERR:
		s = Impact(false, false);
		break;
	case Packet::STATE::CONNECTED:
		s = NetworkInterface::STATUS::NOTHING;
		Graphics::ConnectionStatus = Graphics::CONNECTIONSTATUS::CONNECTED;
		break;
	case Packet::STATE::ERR:
		s = NetworkInterface::STATUS::NEEDCLEAN;
		Graphics::ConnectionStatus = Graphics::CONNECTIONSTATUS::DISCONNECTED;
		Message("Ошибка.", errStr);
		ExitGame(true);
		break;
	case Packet::STATE::DISCONNECTED:
		s = Impact(true, true);
		break;
	case Packet::STATE::BUSY:
		s = NetworkInterface::STATUS::NEEDCLEAN;
		Message("Сервер уже участвует в сражении.", "Повторите попытку позже.");
		Impact(true, false);
		break;
	default:
		throw exception(__func__);
	}
	update();
	if (status)
		*status = s;
}

tuple<Ship::TYPES, Ship::ROTATE> SeaBattle::GetSelectedShip() const
{
	if (_graphics.IsReadyToPlay())
		return make_tuple(Ship::TYPES::EMPTY, Ship::ROTATE::NIL);
	Ship::TYPES ship;
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
		return make_tuple(Ship::TYPES::EMPTY, Ship::ROTATE::NIL);
	}
	Ship::ROTATE rotate;
	switch (_mainForm.lstDirection->currentRow())
	{
	case 0:
		rotate = Ship::ROTATE::STARTRIGHT;
		break;
	case 1:
		rotate = Ship::ROTATE::STARTDOWN;
		break;
	default:
		return make_tuple(Ship::TYPES::EMPTY, Ship::ROTATE::NIL);
	}
	return make_tuple(ship, rotate);
}

void SeaBattle::AddShip()
{
	if (const auto selShip = GetSelectedShip();
		get<0>(selShip) != Ship::TYPES::EMPTY &&
		_graphics.AddShip(get<0>(selShip), get<1>(selShip)) == Graphics::SHIPADDITION::OK)
		RenewShipCount();
}

void SeaBattle::RemoveShip()
{
	if (_graphics.RemoveShip() != Graphics::SHIPADDITION::OK)
		return;
	RenewShipCount();
	update();
}

void SeaBattle::RenewShipCount() const
{
	const auto f = [this](const Ship::TYPES shipType, QListWidgetItem* const item, const int n, const int maxCount)
	{
		QString str = item->text();
		const int shipCount = _graphics.GetShipCount(shipType);
		str[n] = QString::number(shipCount)[0];
		item->setText(str);
		if (shipCount < maxCount)
			return;
		const int r = _mainForm.lstShipArea->currentRow();
		if (_graphics.GetShipCount(Ship::TYPES::LINKOR) == Ship::GetMaxShipCount(Ship::TYPES::LINKOR) &&
			_graphics.GetShipCount(Ship::TYPES::CRUISER) == Ship::GetMaxShipCount(Ship::TYPES::CRUISER) &&
			_graphics.GetShipCount(Ship::TYPES::ESMINEC) == Ship::GetMaxShipCount(Ship::TYPES::ESMINEC) &&
			_graphics.GetShipCount(Ship::TYPES::VEDETTE) == Ship::GetMaxShipCount(Ship::TYPES::VEDETTE))
			return;
		switch (shipType)
		{
		case Ship::TYPES::LINKOR:
			if (r != 0)
				return;
			break;
		case Ship::TYPES::CRUISER:
			if (r != 1)
				return;
			break;
		case Ship::TYPES::ESMINEC:
			if (r != 2)
				return;
			break;
		case Ship::TYPES::VEDETTE:
			if (_graphics.GetShipCount(Ship::TYPES::LINKOR) < Ship::GetMaxShipCount(Ship::TYPES::LINKOR))
				_mainForm.lstShipArea->setCurrentRow(0);
			else
				if (_graphics.GetShipCount(Ship::TYPES::CRUISER) < Ship::GetMaxShipCount(Ship::TYPES::CRUISER))
					_mainForm.lstShipArea->setCurrentRow(1);
				else
					if (_graphics.GetShipCount(Ship::TYPES::ESMINEC) < Ship::GetMaxShipCount(Ship::TYPES::ESMINEC))
						_mainForm.lstShipArea->setCurrentRow(2);
					else
						if (_graphics.GetShipCount(Ship::TYPES::VEDETTE) < Ship::GetMaxShipCount(Ship::TYPES::VEDETTE))
							_mainForm.lstShipArea->setCurrentRow(3);
			return;
		case Ship::TYPES::EMPTY:
			return;
		default:
			throw exception("RenewShipCount");
		}
		if ((r >= 0) && r < (_mainForm.lstShipArea->count() - 1))
			_mainForm.lstShipArea->setCurrentRow(r + 1);
	};

	f(Ship::TYPES::LINKOR, _mainForm.lstShipArea->item(0), 41, Ship::GetMaxShipCount(Ship::TYPES::LINKOR));
	f(Ship::TYPES::CRUISER, _mainForm.lstShipArea->item(1), 45, Ship::GetMaxShipCount(Ship::TYPES::CRUISER));
	f(Ship::TYPES::ESMINEC, _mainForm.lstShipArea->item(2), 44, Ship::GetMaxShipCount(Ship::TYPES::ESMINEC));
	f(Ship::TYPES::VEDETTE, _mainForm.lstShipArea->item(3), 35, Ship::GetMaxShipCount(Ship::TYPES::VEDETTE));
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
		if (Graphics::ConnectionStatus == Graphics::CONNECTIONSTATUS::CONNECTED)
		{
			if (const optional<QString> s = _clientServer->SendHit())
				Message("Сюда ударить нельзя.", *s, QMessageBox::Information);
		}
		else
			AddShip();
		break;
	case Qt::RightButton:
		RemoveShip();
		break;
	default:
		QWidget::mouseReleaseEvent(event);
		return;
	}
	update();
}

void SeaBattle::keyReleaseEvent(QKeyEvent* event)
{
	if (event->isAutoRepeat())
	{
		QWidget::keyReleaseEvent(event);
		return;
	}
	switch (event->key())
	{
	case Qt::Key::Key_Escape:
		if (close())
			QApplication::quit();
		return;
	case Qt::Key::Key_Delete:
		RemoveShip();
		return;
	default:
		QWidget::keyReleaseEvent(event);
	}
}

void SeaBattle::closeEvent(QCloseEvent* event)
{
	if (Graphics::ConnectionStatus == Graphics::CONNECTIONSTATUS::CONNECTED)
		switch (Message("Партия не закончена.", "Выйти из игры?", QMessageBox::Question, QMessageBox::Yes | QMessageBox::No, QMessageBox::No, QMessageBox::No))
		{
		case QMessageBox::Yes:
			break;
		case QMessageBox::No:
			event->ignore();
			return;
		default:
			throw exception(__func__);
		}
	SaveParameters();
	event->accept();
}

optional<quint16> SeaBattle::GetPort() const
{
	bool ok;
	const auto port = static_cast<quint16>(_mainForm.txtPort->text().toUInt(&ok));
	if (ok)
		return port;
	return nullopt;
}

QMessageBox::StandardButton SeaBattle::Message(const QString& situation, const QString& question, const QMessageBox::Icon icon, const QMessageBox::StandardButtons btnSet, const QMessageBox::StandardButton btnDef, const QMessageBox::StandardButton btnEsc)
{
	MyMessageBox msgBox(this);
	msgBox.setText(situation);
	msgBox.setInformativeText(question);
	msgBox.setIcon(icon);
	msgBox.setStandardButtons(btnSet);
	msgBox.setDefaultButton(btnDef);
	msgBox.setEscapeButton(btnEsc);
	if (btnSet & QMessageBox::No)
		msgBox.setButtonText(QMessageBox::No, "Нет");
	if (btnSet & QMessageBox::Yes)
		msgBox.setButtonText(QMessageBox::Yes, "Да");
	return static_cast<QMessageBox::StandardButton>(msgBox.exec());
}

void SeaBattle::SlotMessage(const QString situation, const QString question, const qint32 icon, const bool clearBit)
{
	Message(situation, question, static_cast<QMessageBox::Icon>(icon));
	ExitGame(clearBit);
}

void SeaBattle::SlotGrab()
{
	const QPixmap p = grab(QRect(QPoint(0, 0), QSize(1, 1)));
	_neutralColor = p.toImage().pixelColor(0, 0);
}

NetworkInterface::STATUS SeaBattle::Impact(const bool disconnect, const bool disconnectMessage)
{
	switch (_graphics.GetBroken())
	{
	case Graphics::BROKEN::ME:
		emit SigMessage("Поражение!", "Вы проиграли.", QMessageBox::Information, false);
		return NetworkInterface::STATUS::NEEDCLEAN;
	case Graphics::BROKEN::RIVAL:
		emit SigMessage("Победа!", "Соперник потерпел поражение.", QMessageBox::Information, false);
		return NetworkInterface::STATUS::NEEDCLEAN;
	case Graphics::BROKEN::NOTHING:
		if (!disconnect)
			return NetworkInterface::STATUS::NOTHING;
		if (disconnectMessage)
			emit SigMessage("Игра прекращена.", "Соединение разорвано.", QMessageBox::Critical, true);
		else
			ExitGame(true);
		return NetworkInterface::STATUS::NEEDCLEAN;
	default:
		throw exception(__func__);
	}
}
