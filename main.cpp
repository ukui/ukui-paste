#include "mainwindow.h"

#include <QApplication>
#include <SingleApplication>

#ifdef Q_OS_LINUX
#include <KF5/KWindowSystem/KWindowEffects>
#endif

int main(int argc, char *argv[])
{
	SingleApplication a(argc, argv);

	MainWindow w;
	QObject::connect(&a, &SingleApplication::instanceStarted, [&w](void) {
		w.hide();
	});

	a.setQuitOnLastWindowClosed(false);
#ifdef Q_OS_LINUX
	KWindowEffects::enableBlurBehind(w.winId(), true);
#endif
	return a.exec();
}
