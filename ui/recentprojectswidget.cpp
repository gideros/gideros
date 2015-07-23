#include "recentprojectswidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QSettings>
#include <QFileInfo>
#include <QFont>
#include <QFontMetrics>
#include <QDir>

RecentProjectsWidget::RecentProjectsWidget(QWidget *parent) :
    QWidget(parent)
{
	width_ = 240;
	height_ = 330;

	QSettings settings;
	QStringList files = settings.value("recentProjectList").toStringList();

	int numRecentFiles = qMin(files.size(), 7);
	items_.resize(numRecentFiles);
	for (int i = 0; i < numRecentFiles; ++i)
	{
		items_[i].name = QFileInfo(files[i]).baseName();
		items_[i].path = files[i];
	}

	selected_ = -1;
}

void RecentProjectsWidget::paintEvent(QPaintEvent* event)
{
	QPainter painter(this);
//	painter.setBackground(QColor(0xFAFAFA));
//	painter.eraseRect(rect());

#if defined(Q_OS_MAC)
	QFont title("Tahoma", 12+1, QFont::Bold, false);
	QFont bold("Tahoma", 10+1, QFont::Bold, false);
	QFont normal("Tahoma", 8+1);
#else
	QFont title("Tahoma", 12, QFont::Bold, false);
	QFont bold("Tahoma", 10, QFont::Bold, false);
	QFont normal("Tahoma", 8);
#endif

	painter.setPen(QColor(0x808080));
	painter.setBrush(QColor(0xFAFAFA));
	painter.drawRoundedRect(0, 0, width_, height_, 8, 8);

	painter.setPen(Qt::NoPen);
	painter.setBrush(QColor(0xE8E8E8));
//	painter.drawRect(1, 10, width_ - 1, 10);

	painter.setPen(QColor(0x808080));
	painter.drawLine(0, 20, width_, 20);

	painter.setPen(Qt::black);
	painter.setFont(bold);
	painter.drawText(10, 0, width_, 20, Qt::AlignVCenter, "Recent Projects");

	for (size_t i = 0; i < items_.size(); ++i)
	{
		int y = i * 40;
		int x = 0;

		if (i == selected_)
		{
			painter.setBrush(QBrush(0x3875D7));
			painter.setPen(Qt::NoPen);
			painter.drawRect(x+1, y + 21, width_ - 1, 40);

			painter.setPen(Qt::white);
			painter.setFont(bold);
			painter.drawText(x + 10, y + 24, width_ - 10 - 5, 40, 0, items_[i].name);

			painter.setPen(Qt::white);
			painter.setFont(normal);
			QFontMetrics fm(normal);
			QString path = QFileInfo(items_[i].path).dir().absolutePath();
			path = fm.elidedText(path, Qt::ElideMiddle, width_ - 10 - 5);
			painter.drawText(x + 10, y + 40, width_ - 10 - 5, 40, 0, path);
		}
		else
		{
			painter.setPen(Qt::black);
			painter.setFont(bold);
			painter.drawText(x + 10, y + 24, width_ - 10 - 5, 40, 0, items_[i].name);

			painter.setPen(QColor(0x808080));
			painter.setFont(normal);
			QFontMetrics fm(normal);
			QString path = QFileInfo(items_[i].path).dir().absolutePath();
			path = fm.elidedText(path, Qt::ElideMiddle, width_ - 10 - 5);
			painter.drawText(x + 10, y + 40, width_ - 10 - 5, 40, 0, path);
		}
	}

	QWidget::paintEvent(event);
}

void RecentProjectsWidget::mousePressEvent(QMouseEvent* event)
{
	selected_ = itemAt(event->x(), event->y());
	update();
	emit selected(selected_);
}

int RecentProjectsWidget::itemAt(int mx, int my)
{
	for (size_t i = 0; i < items_.size(); ++i)
	{
		int y = i * 40;
		int x = 0;

		QRect rect(x + 1, y + 21, width_ - 1, 40);

		if (rect.contains(mx, my))
			return i;
	}

	return -1;
}

void RecentProjectsWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
	int item = itemAt(event->x(), event->y());
	if (item != -1)
		emit openProject(items_[item].path);
}

void RecentProjectsWidget::clearSelection()
{
	selected_ = -1;
	update();
}
