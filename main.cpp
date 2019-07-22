#include "stdafx.h"
#include "SeaBattle.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	SeaBattle w;
	w.show();
	return a.exec();
}
