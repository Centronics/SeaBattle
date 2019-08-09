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

private slots:

	void SlotBtnClearShipsClicked();
	void SlotBtnConnectClicked();
	void SlotBtnServerStartClicked();
	void SlotBtnDisconnectClicked();
	void SlotReceive(const Packet& packet);

public:

	explicit SeaBattle(QWidget* parent = Q_NULLPTR);
	SeaBattle(const SeaBattle&) = delete;
	SeaBattle(SeaBattle&&) = delete;
	~SeaBattle() = default;
	SeaBattle& operator=(const SeaBattle&) = delete;
	SeaBattle& operator=(SeaBattle&&) = delete;

protected:

	bool CheckGameReady();
	void paintEvent(QPaintEvent* event) override;
	void AddShip();
	void RenewShipCount() const;
	void mouseMoveEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
	void keyReleaseEvent(QKeyEvent* event) override;
	void Message(const QString& comment, const QString& infoMessage);
	void Impact();
	[[nodiscard]] std::tuple<Ship::SHIPTYPES, Ship::ROTATE, QListWidgetItem*> GetSelectedShip() const;
	[[nodiscard]] std::optional<quint16> GetPort();

	template<typename T> T* Initialize()
	{
		const auto f = [this]() -> T*
		{
			T* const result = new T(_graphics, *this, this);
			connect(result, SIGNAL(SignalReceive(const Packet&)), this, SLOT(SlotReceive(const Packet&)));
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
