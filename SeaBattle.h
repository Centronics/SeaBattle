#pragma once
#include "ui_SeaBattle.h"
#include "Graphics.h"
#include "NetworkInterface.h"
#include "Help.h"

class SeaBattle : public QWidget
{
	Q_OBJECT

	Ui::SeaBattleForm _mainForm;
	HelpForm _helpForm{ this };
	Graphics _graphics;
	NetworkInterface* _clientServer = nullptr;
	inline static const QString SettingsFileName = "Settings.xml";

private slots:

	void SlotBtnHelpClicked();
	void SlotBtnConnectClicked();
	void SlotBtnServerStartClicked();
	void SlotBtnDisconnectClicked();
	void SlotReceive(Packet packet);

signals:

	void SignalClose();

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
	void RenewShipCount() const;
	void mouseMoveEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
	void keyReleaseEvent(QKeyEvent* event) override;
	void closeEvent(QCloseEvent* event) override;
	void paintEvent(QPaintEvent* event) override;
	QMessageBox::StandardButton Message(const QString& situation, const QString& question, QMessageBox::Icon icon = QMessageBox::Icon::Critical, QMessageBox::StandardButtons btnSet = QMessageBox::Ok, QMessageBox::StandardButton btnDef = QMessageBox::Ok, QMessageBox::StandardButton btnEsc = QMessageBox::Ok);
	void Impact(bool disconnect, bool disconnectMessage = true);
	void SaveParameters() const;
	void LoadParameters() const;
	void ExitGame();
	[[nodiscard]] std::tuple<Ship::TYPES, Ship::ROTATE, QListWidgetItem*> GetSelectedShip() const;
	[[nodiscard]] std::optional<quint16> GetPort() const;

	template<typename T> T* Initialize()
	{
		T* const result = new T(_graphics, this, &_clientServer);
		connect(result, SIGNAL(SignalReceive(Packet)), SLOT(SlotReceive(Packet)));
		connect(this, SIGNAL(SignalClose()), result, SLOT(SlotClose()));
		connect(result, SIGNAL(Update()), SLOT(update()));
		_clientServer = result;
		return reinterpret_cast<T*>(_clientServer);
	}
};
