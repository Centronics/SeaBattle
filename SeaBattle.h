#pragma once
#include "ui_SeaBattle.h"
#include "Graphics.h"
#include "Packet.h"
#include "NetworkInterface.h"
#include "Client.h"

class SeaBattle : public QWidget
{
	Q_OBJECT

	Ui::SeaBattleForm _mainForm{ };
	Graphics _graphics;
	std::unique_ptr<NetworkInterface> _clientServer{ };
	inline static const QString SettingsFileName = "Settings.xml";

private slots:

	void SlotBtnClearShipsClicked();
	void SlotBtnConnectClicked();
	void SlotBtnServerStartClicked();
	void SlotBtnDisconnectClicked();
	void SlotReceive(const Packet&);

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
	QMessageBox::StandardButton Message(const QString& comment, const QString& infoMessage, QMessageBox::Icon icon = QMessageBox::Icon::Critical, QMessageBox::StandardButtons btnSet = QMessageBox::Ok, QMessageBox::StandardButton btnDef = QMessageBox::Ok, QMessageBox::StandardButton btnEsc = QMessageBox::Ok);
	void Impact(bool disconnect);
	void SaveParameters() const;
	void LoadParameters() const;
	[[nodiscard]] std::tuple<Ship::TYPES, Ship::ROTATE, QListWidgetItem*> GetSelectedShip() const;
	[[nodiscard]] std::optional<quint16> GetPort();

	template<typename T> T* Initialize()
	{
		const auto f = [this]() -> T*
		{
			T* const result = new T(_graphics, *this, this);
			connect(result, SIGNAL(SignalReceive(const Packet&)), SLOT(SlotReceive(const Packet&)));
			return result;
		};

		if (!_clientServer)
			_clientServer.reset(f());
		else
			if (!dynamic_cast<T*>(_clientServer.get()))
				_clientServer.reset(f());
		return reinterpret_cast<T*>(_clientServer.get());
	}
};
