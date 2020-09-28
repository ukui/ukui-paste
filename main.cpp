#include <QApplication>
#include <QGuiApplication>
#include <SingleApplication>
#include <QTranslator>
#include <QLocale>

#include "mainwindow.h"

#ifdef Q_OS_LINUX

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

void LoadTranlateFile(SingleApplication *app)
{
	QTranslator *translator = new QTranslator;

	QLocale locale = QLocale::system();
	if (locale.language() == QLocale::Chinese) {
		if (!translator->load(QString(QM_FILES_INSTALL_PATH)+"/Pastes_zh_CN.qm"))
			translator->load("Pastes_zh_CN.qm");
		app->installTranslator(translator);
	}
}

int main(int argc, char *argv[])
{
#ifdef Q_OS_LINUX
	if (getScreenWidth() > 2560) {
		QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
		QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
	}
#endif
	QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
	QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

	SingleApplication a(argc, argv);
	LoadTranlateFile(&a);

	MainWindow w;
	QObject::connect(&a, &SingleApplication::instanceStarted, [&w](void) {
		w.hide();
	});

	a.setQuitOnLastWindowClosed(false);
	return a.exec();
}
