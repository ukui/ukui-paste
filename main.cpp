#include <QApplication>
#include <QGuiApplication>
#include <SingleApplication>

#include "mainwindow.h"

#ifdef Q_OS_LINUX
#include <X11/Xlib.h>

static int getScreenWidth(void)
{
	Display *display;
	Screen *screen;
	int width = 0;

	display = XOpenDisplay(NULL);
	if (!display)
		return 0;

	screen = DefaultScreenOfDisplay(display);
	if (!screen)
		goto out;

	width = screen->width;
out:
	XCloseDisplay(display);

	return width;
}

#endif

int main(int argc, char *argv[])
{
#ifdef Q_OS_LINUX
	if (getScreenWidth() > 2560) {
		QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
		QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
	}
#endif

	SingleApplication a(argc, argv);

	MainWindow w;
	QObject::connect(&a, &SingleApplication::instanceStarted, [&w](void) {
		w.hide();
	});

	a.setQuitOnLastWindowClosed(false);
	return a.exec();
}
