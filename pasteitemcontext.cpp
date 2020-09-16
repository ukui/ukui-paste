#include <gio/gdesktopappinfo.h>

#include <QResizeEvent>
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

QString FileFrame::getFileIconName(const QString &uri)
{
	QString icon_name;

#ifdef Q_OS_LINUX
	auto file = g_file_new_for_path(uri.toLocal8Bit());
	auto info = g_file_query_info(file,
				      G_FILE_ATTRIBUTE_THUMBNAIL_PATH ","
				      G_FILE_ATTRIBUTE_THUMBNAILING_FAILED ","
				      G_FILE_ATTRIBUTE_STANDARD_ICON,
				      G_FILE_QUERY_INFO_NONE,
				      nullptr,
				      nullptr);
	if (!G_IS_FILE_INFO (info))
		return nullptr;
	GIcon *g_icon = g_file_info_get_icon (info);
	//do not unref the GIcon from info.
	if (G_IS_ICON(g_icon)) {
		const gchar* const* icon_names = g_themed_icon_get_names(G_THEMED_ICON (g_icon));
		if (icon_names)
			icon_name = QString (*icon_names);
	}
#endif
	return icon_name;
}

void FileFrame::setUrls(QList<QUrl> &urls)
{
	for (int i = 0; i < urls.count() && i < 3; i++) {
		auto url = urls.at(i);
		auto icon = QIcon::fromTheme(getFileIconName(url.toLocalFile()), QIcon::fromTheme("text-x-generic"));
		QLabel *label = new QLabel(this);
		label->setPixmap(icon.pixmap(100, 100));
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
