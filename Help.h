#pragma once
#include "ui_SeaBattleHelp.h"

class HelpForm : public QDialog
{
	Q_OBJECT

	Ui::Help _helpForm;

public:

	explicit HelpForm(QWidget* parent = Q_NULLPTR) noexcept : QDialog(parent)
	{
		_helpForm.setupUi(this);
		_helpForm.textBrowser->setSource(QUrl("qrc:/Help.html"));
		setWindowTitle(_helpForm.textBrowser->documentTitle());
	}

	void keyPressEvent(QKeyEvent* event) override
	{
		event->accept();
	}

	void keyReleaseEvent(QKeyEvent* event) override
	{
		if (!event->isAutoRepeat() && event->key() == Qt::Key::Key_Escape)
			close();
	}
};