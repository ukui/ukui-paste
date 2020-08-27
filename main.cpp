#include "mainwindow.h"

#include <QApplication>
#include <SingleApplication>

int main(int argc, char *argv[])
{
	SingleApplication a(argc, argv);

	MainWindow w;
	QObject::connect(&a, &SingleApplication::instanceStarted, [&w](void) {
		w.hide();
	});

	return a.exec();
}
