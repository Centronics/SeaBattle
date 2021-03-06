#pragma once
#include "ui_SeaBattle.h"
#include "Graphics.h"
#include "NetworkInterface.h"
#include "Help.h"

class SeaBattle : public QWidget
{
	Q_OBJECT

	Ui::SeaBattleForm _mainForm{};
	HelpForm _helpForm{ this };
	Graphics _graphics;
	NetworkInterface* _clientServer = nullptr;
	QColor _neutralColor;
	inline static const QString SettingsFileName = "Settings.xml";

public:

	explicit SeaBattle(QWidget* parent = Q_NULLPTR) noexcept;
	SeaBattle(const SeaBattle&) = delete;
	SeaBattle(SeaBattle&&) = delete;
	~SeaBattle() = default;
	SeaBattle& operator=(const SeaBattle&) = delete;
	SeaBattle& operator=(SeaBattle&&) = delete;

protected:

	bool eventFilter(QObject* watched, QEvent* event) override;
	void OffButtons(bool off = true) const;
	bool CheckGameReady();
	void AddShip();
	void RemoveShip();
	void UpdateShipCount() const;
	void mouseMoveEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
	void keyReleaseEvent(QKeyEvent* event) override;
	void closeEvent(QCloseEvent* event) override;
	void paintEvent(QPaintEvent* event) override;
	QMessageBox::StandardButton Message(const QString& situation, const QString& question, QMessageBox::Icon icon = QMessageBox::Icon::Critical, QMessageBox::StandardButtons btnSet = QMessageBox::Ok, QMessageBox::StandardButton btnDef = QMessageBox::Ok, QMessageBox::StandardButton btnEsc = QMessageBox::Ok);
	NetworkInterface::STATUS Impact(bool disconnect, bool disconnectMessage);
	void SaveParameters() const;
	void LoadParameters() const;
	void ExitGame(bool clearBit);
	[[nodiscard]] std::tuple<Ship::TYPES, Ship::ROTATE> GetSelectedShip() const;
	[[nodiscard]] std::optional<quint16> GetPort() const;

	template<typename T> T* Initialize()
	{
		_clientServer->Close();
		T* const result = new T(_graphics, this, &_clientServer);
		Q_UNUSED(connect(result, SIGNAL(SigReceive(Packet, NetworkInterface::STATUS*)), SLOT(SlotReceive(Packet, NetworkInterface::STATUS*))));
		Q_UNUSED(connect(result, SIGNAL(SigUpdate()), SLOT(update())));
		_clientServer = result;
		return reinterpret_cast<T*>(_clientServer);
	}

private slots:

	void SlotLstChange(int currentRow);
	void SlotBtnHelpClicked();
	void SlotBtnConnectClicked();
	void SlotBtnServerStartClicked();
	void SlotBtnDisconnectClicked();
	void SlotReceive(Packet packet, NetworkInterface::STATUS* status);
	void SlotMessage(QString situation, QString question, qint32 icon, bool clearBit);
	void SlotGrab();

signals:

	void SigMessage(QString, QString, qint32, bool);
	void SigGrab();
};
