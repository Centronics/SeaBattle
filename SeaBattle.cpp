#include "stdafx.h"
#include "SeaBattle.h"
#include "Packet.h"
#include "Server.h"
#include "MyMessageBox.h"
#include "Client.h"

using namespace std;

SeaBattle::SeaBattle(QWidget* parent) noexcept : QWidget(parent)
{
	_mainForm.setupUi(this);
	_mainForm.frmDrawing->installEventFilter(this);
	setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint | Qt::MSWindowsFixedSizeDialogHint);
	_mainForm.lstShipArea->setCurrentRow(0);
	_mainForm.lstDirection->setCurrentRow(0);
	LoadParameters();
	connect(_mainForm.btnHelp, SIGNAL(clicked()), SLOT(SlotBtnHelpClicked()));
	connect(_mainForm.btnConnect, SIGNAL(clicked()), SLOT(SlotBtnConnectClicked()));
	connect(_mainForm.btnServerStart, SIGNAL(clicked()), SLOT(SlotBtnServerStartClicked()));
	connect(_mainForm.btnDisconnect, SIGNAL(clicked()), SLOT(SlotBtnDisconnectClicked()));
	_helpForm.setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);
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
		Message("Порт не указан.", "Укажите порт, к которому необходимо подключиться!");
		return;
	}
	OffButtons();
	Graphics::ConnectionStatus = Graphics::CONNECTIONSTATUS::CLIENT;
	Graphics::IsRivalMove = false;
	Initialize<Client>()->Connect(_mainForm.txtIPAddress->text(), *port);//Убрать в отдельный поток
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
		Message("Порт не указан.", "Укажите порт, который необходимо прослушивать!");
		return;
	}
	OffButtons();
	Graphics::ConnectionStatus = Graphics::CONNECTIONSTATUS::SERVER;
	Graphics::IsRivalMove = true;
	Initialize<Server>()->Listen(*port);//Убрать в отдельный поток
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
	if (watched != _mainForm.frmDrawing)
		return QWidget::eventFilter(watched, event);
	switch (event->type())
	{
	case QEvent::Leave:
		Graphics::CursorX = -1;
		Graphics::CursorY = -1;
		_mainForm.frmDrawing->update();
		QWidget::eventFilter(watched, event);
		return true;
	case QEvent::Paint:
	{
		QPainter painter(_mainForm.frmDrawing);
		const auto selShip = GetSelectedShip();
		_graphics.Paint(painter, get<0>(selShip), get<1>(selShip));
		QWidget::eventFilter(watched, event);
		return true;
	}
	default:
		return QWidget::eventFilter(watched, event);
	}
}

void SeaBattle::paintEvent(QPaintEvent*)
{
	QPainter painter(this);
	Graphics::DrawMoveQuad(painter);
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
	ExitGame();
	update();
}

void SeaBattle::ExitGame()
{
	OffButtons(false);
	// Исправить баг с отображением соперников в случае проигрыша при наличии промахов; Найден баг, что если произошла ошибка при открытии сервера (например), то сообщение о поражении (победе) повтоярется после неё. ПРОТЕСТИРОВАТЬ ситуацию, когда пытаются подключиться более одного клиента.
	_clientServer->Close();
	//emit SignalClose();
	Graphics::ConnectionStatus = Graphics::CONNECTIONSTATUS::DISCONNECTED;
	Graphics::IsRivalMove = false;
}

void SeaBattle::SlotReceive(const Packet packet)  // NOLINT(performance-unnecessary-value-param)
{
	switch (QString errStr; packet.GetState(&errStr))
	{
	case Packet::STATE::CONNECTED:
		Graphics::ConnectionStatus = Graphics::CONNECTIONSTATUS::CONNECTED;
		break;
	case Packet::STATE::ERR:
		Graphics::ConnectionStatus = Graphics::CONNECTIONSTATUS::DISCONNECTED;
		Message("Ошибка.", errStr);
		ExitGame();
		break;
	case Packet::STATE::DISCONNECTED:
		//Impact(true, false);
		break;
	case Packet::STATE::BUSY:
		Message("Сервер уже участвует в сражении.", "Повторите попытку позже.");
		//Impact(true);
		break;
	default:
		throw exception(__func__);
	}
	update();
}

tuple<Ship::TYPES, Ship::ROTATE, QListWidgetItem*> SeaBattle::GetSelectedShip() const
{
	if (_graphics.IsReadyToPlay())
		return make_tuple(Ship::TYPES::EMPTY, Ship::ROTATE::NIL, nullptr);
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
		return make_tuple(Ship::TYPES::EMPTY, Ship::ROTATE::NIL, nullptr);
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

void SeaBattle::RemoveShip()
{
	switch (_graphics.RemoveShip())
	{
	case Graphics::SHIPADDITION::OK:
		RenewShipCount();
		update();
		return;
	case Graphics::SHIPADDITION::NOCOORD:
		return;
	case Graphics::SHIPADDITION::INCORRECTMODE:
		Message("Неверный режим.", "Добавлять или удалять корабли можно только до начала игры.");
		return;
	default:
		throw exception(__func__);
	}
}

void SeaBattle::RenewShipCount() const
{
	const auto f = [this](const Ship::TYPES shipType, QListWidgetItem* const item, const int n)
	{
		QString str = item->text();
		str[n] = QString::number(_graphics.GetShipCount(shipType))[0];
		item->setText(str);
	};

	f(Ship::TYPES::LINKOR, _mainForm.lstShipArea->item(0), 41);
	f(Ship::TYPES::CRUISER, _mainForm.lstShipArea->item(1), 45);
	f(Ship::TYPES::ESMINEC, _mainForm.lstShipArea->item(2), 44);
	f(Ship::TYPES::VEDETTE, _mainForm.lstShipArea->item(3), 35);
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
			if (const std::optional<QString> s = _clientServer->SendHit())
				Message("Сюда ударить нельзя.", *s, QMessageBox::Information);
			else
				Impact(false);
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
	if (Graphics::ConnectionStatus != Graphics::CONNECTIONSTATUS::DISCONNECTED)
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

void SeaBattle::Impact(const bool disconnect, const bool disconnectMessage)
{
	switch (_graphics.GetBroken())
	{
	case Graphics::BROKEN::ME:
		ExitGame();
		Message("Поражение!", "Вы проиграли.", QMessageBox::Information);
		return;
	case Graphics::BROKEN::RIVAL:
		ExitGame();
		Message("Победа!", "Соперник потерпел поражение.", QMessageBox::Information);
		return;
	case Graphics::BROKEN::NOTHING:
		if (!disconnect)
			return;
		ExitGame();
		if (disconnectMessage)
			Message("Игра прекращена.", "Соединение разорвано.");
		return;
	default:
		throw exception(__func__);
	}
}
