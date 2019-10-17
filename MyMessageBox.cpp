#include "stdafx.h"
#include "MyMessageBox.h"

MyMessageBox::MyMessageBox(QWidget* parent) : QMessageBox(parent)
{
	setWindowTitle("Морской бой");
	setWindowModality(Qt::WindowModal);
	setTextFormat(Qt::PlainText);
	setTextInteractionFlags(Qt::NoTextInteraction);
	setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint | Qt::MSWindowsFixedSizeDialogHint);
	setStyleSheet("QMessageBox { font-family: Arial; font-style: normal; font-size: 15pt; color: #000000; }");
}

void MyMessageBox::keyPressEvent(QKeyEvent* event)
{
	event->accept();
}

void MyMessageBox::keyReleaseEvent(QKeyEvent* event)
{
	if (!event->isAutoRepeat() && event->key() == Qt::Key::Key_Escape)
		close();
}
