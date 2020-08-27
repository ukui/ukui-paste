#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	MainWindow w;
	w.show();
	w.setFocus();
	w.activateWindow();

	return a.exec();
}
