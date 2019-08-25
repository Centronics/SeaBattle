#include "stdafx.h"
#include "SeaBattle.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	SeaBattle w;
	w.show();
	return QApplication::exec();
}
