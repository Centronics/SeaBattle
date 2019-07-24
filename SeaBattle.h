#pragma once
#include "ui_SeaBattle.h"
#include "Graphics.h"
#include "Packet.h"
#include "ClientServer.h"

class SeaBattle : public QWidget
{
	Q_OBJECT

private slots:

	void BtnClearShips();
	void BtnConnectClicked();
	void BtnServerStartClicked();
	bool CheckGameReady();
	void BtnDisconnect();

public:

	explicit SeaBattle(QWidget *parent = Q_NULLPTR);
	SeaBattle(const SeaBattle&) = delete;
	SeaBattle(SeaBattle&&) = delete;
	~SeaBattle() = default;
	SeaBattle& operator=(const SeaBattle&) = delete;
	SeaBattle& operator=(SeaBattle&&) = delete;

protected:

	void paintEvent(QPaintEvent *event) override;
	[[nodiscard]] std::tuple<bool, Ship::SHIPS, Ship::STATE, QListWidgetItem*> GetSelectedShip() const;
	void AddShip();
	void RenewShipCount() const;
	void mouseMoveEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
	void keyReleaseEvent(QKeyEvent* event) override;
	[[nodiscard]] std::optional<int> GetPort();
	void Message(const QString& m1, const QString& infoMessage);

private:

	Ui::SeaBattleForm _mainForm{};
	Graphics _graphics;
	ClientServer _clientServer{ _graphics, *this };
	Packet _packet;
};