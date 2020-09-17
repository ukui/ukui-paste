#include <qsystemdetection.h>

#ifdef Q_OS_LINUX
#include <gio/gdesktopappinfo.h>
#endif

#include <QResizeEvent>
#include <QFileInfo>
#include <QPixmap>
#include <QDebug>

#include "pasteitemcontext.h"

TextFrame::TextFrame(QWidget *parent) : QLabel(parent),
	m_mask_label(new QLabel(this))
{
	this->setObjectName("ContextTextFrame");
	this->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	this->setWordWrap(true);

	this->m_mask_label->setAlignment(Qt::AlignCenter);
	this->m_mask_label->setContentsMargins(0, 3, 0, 3);
}

TextFrame::~TextFrame()
{
	delete m_mask_label;
}

void TextFrame::setMaskFrameText(QString s)
{
	this->m_mask_label->setText(s);
	this->m_mask_label->show();
}

void TextFrame::resizeEvent(QResizeEvent *event)
{
	this->m_mask_label->setGeometry(0, this->height()-LABEL_HEIGHT,
					this->width(), LABEL_HEIGHT);

	QLabel::resizeEvent(event);
}

PixmapFrame::PixmapFrame(QWidget *parent) : TextFrame(parent)
{
	this->setObjectName("ContextPixmapFrame");
	this->setAlignment(Qt::AlignCenter);
}

void PixmapFrame::resizeEvent(QResizeEvent *event)
{
	if (!m_pixmap.isNull()) {
		this->setPixmap(m_pixmap.scaled(event->size(),
						Qt::KeepAspectRatio,
						Qt::SmoothTransformation));
	}

	TextFrame::resizeEvent(event);
}

FileFrame::FileFrame(QWidget *parent) : QWidget(parent)
{
}

#ifdef Q_OS_WIN
QPixmap pixmapFromShellImageList(int iImageList, const SHFILEINFO &info)
{
	QPixmap result;
	// For MinGW:
	static const IID iID_IImageList = {0x46eb5926, 0x582e, 0x4017, {0x9f, 0xdf, 0xe8, 0x99, 0x8d, 0xaa, 0x9, 0x50}};

	IImageList *imageList = nullptr;
	if (FAILED(SHGetImageList(iImageList, iID_IImageList, reinterpret_cast<void **>(&imageList))))
		return result;

	HICON hIcon = 0;
	if (SUCCEEDED(imageList->GetIcon(info.iIcon, ILD_TRANSPARENT, &hIcon))) {
		result = QtWin::fromHICON(hIcon);
		DestroyIcon(hIcon);
	}

	return result;
}

QIcon FileFrame::getFileIcon(const QString &filename)
{
	QIcon icon;

	if (!filename.isEmpty()) {
		std::string str = "file";
		int index = filename.lastIndexOf(".");
		if (index >= 0) {
			QString suffix = filename.mid(index);

			str = suffix.toUtf8().constData();
		}

		LPCSTR name = str.c_str();
		SHFILEINFOA info;
		if(SHGetFileInfoA(name,
				  FILE_ATTRIBUTE_NORMAL,
				  &info,
				  sizeof(info),
				  SHGFI_SYSICONINDEX| SHGFI_ICON | SHGFI_USEFILEATTRIBUTES))
		{
			icon = QIcon(QtWin::fromHICON(info.hIcon));
		}
	}

	return icon;
}

QIcon FileFrame::getExecutableIcon(const QString &filename)
{
	QIcon icon;

	if (!filename.isEmpty()) {
		TCHAR filename1[MAX_PATH];
		filename.toWCharArray(filename1);
		QPixmap pixmap;
		SHFILEINFO info;
		ZeroMemory(&info, sizeof(SHFILEINFO));
		unsigned int flags = SHGFI_ICON | SHGFI_SYSICONINDEX | SHGFI_ICONLOCATION |
			SHGFI_OPENICON | SHGFI_USEFILEATTRIBUTES;
		const HRESULT hr = SHGetFileInfo(filename1, 0, &info, sizeof(SHFILEINFO), flags);
		if (FAILED(hr)) {
			pixmap = QtWin::fromHICON(::LoadIcon(0, IDI_APPLICATION));
		} else  {
			pixmap = pixmapFromShellImageList(0x4, info);
			if (pixmap.isNull())
				pixmap = pixmapFromShellImageList(0x2, info);
			if (pixmap.isNull())
				pixmap = QtWin::fromHICON(info.hIcon);
			if (pixmap.isNull())
				pixmap = QtWin::fromHICON(::LoadIcon(0, IDI_APPLICATION));
		}
		icon = QIcon(pixmap);
	}

	return icon;
}

QIcon FileFrame::getDirIcon(const QString &dirname)
{
	QIcon folder_icon;

	SHFILEINFOA info;
	if(SHGetFileInfoA(dirname.toUtf8().constData(),
			  FILE_ATTRIBUTE_DIRECTORY,
			  &info,
			  sizeof(info),
			  SHGFI_SYSICONINDEX | SHGFI_ICON| SHGFI_USEFILEATTRIBUTES))
	{
		folder_icon = QIcon(QtWin::fromHICON(info.hIcon));
	}

	return folder_icon;
}

#endif

QIcon FileFrame::getIcon(const QString &uri)
{
	QString icon_name;

#ifdef Q_OS_LINUX
	if (uri.endsWith(".desktop")) {
		auto _desktop_file = g_desktop_app_info_new_from_filename(uri.toUtf8().constData());
		auto _icon_string = g_desktop_app_info_get_string(_desktop_file, "Icon");
		QIcon icon = QIcon::fromTheme(_icon_string, QIcon::fromTheme("text-x-generic"));
		g_free(_icon_string);
		g_object_unref(_desktop_file);
	} else {
		auto file = g_file_new_for_path(uri.toLocal8Bit());
		auto info = g_file_query_info(file,
					      G_FILE_ATTRIBUTE_THUMBNAIL_PATH ","
					      G_FILE_ATTRIBUTE_THUMBNAILING_FAILED ","
					      G_FILE_ATTRIBUTE_STANDARD_ICON,
					      G_FILE_QUERY_INFO_NONE,
					      nullptr,
					      nullptr);
		if (!G_IS_FILE_INFO (info))
			return QIcon();
		GIcon *g_icon = g_file_info_get_icon (info);
		//do not unref the GIcon from info.
		if (G_IS_ICON(g_icon)) {
			const gchar* const* icon_names = g_themed_icon_get_names(G_THEMED_ICON (g_icon));
			if (icon_names)
				icon_name = QString (*icon_names);
		}

		g_object_unref(info);
		g_object_unref(file);
		return QIcon::fromTheme(icon_name, QIcon::fromTheme("text-x-generic"));
	}
#endif
#ifdef Q_OS_WIN
	QFileInfo fileinfo(uri);
	if (fileinfo.isExecutable())
		return FileFrame::getExecutableIcon(uri);
	else if (fileinfo.isFile())
		return FileFrame::getFileIcon(uri);
	else if (fileinfo.isDir())
		return FileFrame::getDirIcon(uri);
#endif
	return QIcon();
}

void FileFrame::setUrls(QList<QUrl> &urls)
{
	for (int i = 0; i < urls.count() && i < 3; i++) {
		auto url = urls.at(i);
		auto icon = this->getIcon(url.toLocalFile());
		QLabel *label = new QLabel(this);
		label->setPixmap(icon.pixmap(128, 128).scaled(128, 128));
		label->show();
	}
}

StackedWidget::StackedWidget(QWidget *parent) : QStackedWidget(parent),
	m_pixmap_frame(new PixmapFrame(this)),
	m_text_frame(new TextFrame(this)),
	m_richtext_frame(new TextFrame(this)),
	m_file_frame(new FileFrame(this))
{
	this->setObjectName("Context");
	this->addWidget(m_pixmap_frame);
	this->addWidget(m_text_frame);
	this->addWidget(m_richtext_frame);
	this->addWidget(m_file_frame);
}

StackedWidget::~StackedWidget()
{
	delete m_pixmap_frame;
	delete m_text_frame;
	delete m_richtext_frame;
	delete m_file_frame;
}

void StackedWidget::setPixmap(QPixmap &pixmap)
{
	m_pixmap_frame->setStorePixmap(pixmap);
	this->setCurrentIndex(StackedWidget::IMAGE);

	QString s = QString("%1x%2 ").arg(pixmap.width()).arg(pixmap.height()) + QObject::tr("px");
	m_pixmap_frame->setMaskFrameText(s);
}

void StackedWidget::setText(QString &s)
{
	m_text_frame->setText(s);
	m_text_frame->setIndent(4);
	m_text_frame->setMaskFrameText(QString("%1 ").arg(s.count()) + QObject::tr("characters"));
	this->setCurrentIndex(StackedWidget::TEXT);
}

void StackedWidget::setRichText(QString &s, int count)
{
	m_richtext_frame->setText(s);
	m_richtext_frame->setTextFormat(Qt::RichText);
	m_richtext_frame->setMaskFrameText(QString("%1 ").arg(count) + QObject::tr("characters"));
	this->setCurrentIndex(StackedWidget::RICHTEXT);
}

void StackedWidget::setUrls(QList<QUrl> &urls)
{
	m_file_frame->setUrls(urls);
	this->setCurrentIndex(StackedWidget::URLS);
}
