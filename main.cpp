#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	MainWindow *w = new MainWindow();
	w->show();
	w->setFocus();
	w->activateWindow();

	QObject::connect(w, SIGNAL(destroyed()), &a, SLOT(quit()));
	return a.exec();
}
