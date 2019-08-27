#pragma once

class MyMessageBox : public QMessageBox
{
public:
	explicit MyMessageBox(QWidget* parent = nullptr);

protected:
	void keyPressEvent(QKeyEvent* event) override;
	void keyReleaseEvent(QKeyEvent* event) override;
};