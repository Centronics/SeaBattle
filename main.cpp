#include "stdafx.h"
#include "SeaBattle.h"
#include "MyMessageBox.h"

static void ShowMessage(const char* message)
{
	MyMessageBox msgBox;
	msgBox.setText("Произошла неисправимая ошибка, игра будет закрыта.");
	msgBox.setInformativeText("Текст ошибки: " + QString(message));
	msgBox.setIcon(QMessageBox::Critical);
	msgBox.setStandardButtons(QMessageBox::Ok);
	msgBox.setDefaultButton(QMessageBox::Ok);
	msgBox.setEscapeButton(QMessageBox::Ok);
	msgBox.setButtonText(QMessageBox::Ok, "Закрыть");
	msgBox.exec();
}

int main(int argc, char* argv[])
{
	QApplication a(argc, argv);
	try
	{
		SeaBattle w;
		w.show();
		return QApplication::exec();
	}
	catch (const std::exception& ex)
	{
		ShowMessage(ex.what());
		return EXIT_FAILURE;
	}
	catch (...)
	{
		ShowMessage("Неизвестная ошибка.");
		return EXIT_FAILURE;
	}
}
