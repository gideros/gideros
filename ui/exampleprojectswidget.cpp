#include "exampleprojectswidget.h"
#include <QPaintEvent>
#include <QPainter>
#include <QPushButton>
#include <QMouseEvent>
#include <QDomDocument>
#include <QFile>
#include <QDebug>

int ExampleProjectsWidget::page_ = 0;

ExampleProjectsWidget::ExampleProjectsWidget(QWidget *parent) :
    QWidget(parent)
{
	width_ = 240;
	height_ = 330;

	QDomDocument doc;
	QFile file("Resources/start/examples.xml");
	if (file.open(QIODevice::ReadOnly) && doc.setContent(&file))
	{
		file.close();

		QDomElement examplese = doc.documentElement();

		QDomNode iter = examplese.firstChild();
		while(!iter.isNull())
		{
			QDomElement categorye = iter.toElement();
			if(!categorye.isNull())
			{
				Category category;
				category.name = categorye.attribute("name");

				QDomNode iter2 = categorye.firstChild();

				while(!iter2.isNull())
				{
					QDomElement examplee = iter2.toElement();

					Item example;

					example.name = examplee.attribute("name");
					example.description = examplee.attribute("description");
#if defined(Q_OS_MAC)
					example.path = "../../Examples/" + examplee.attribute("path");
#else
					example.path = "Examples/" + examplee.attribute("path");
#endif
					example.imagePath = "Resources/start/images/" + examplee.attribute("image");

					category.items.push_back(example);

					iter2 = iter2.nextSibling();
				}


				categories_.push_back(category);
			}

			iter = iter.nextSibling();
		}
	}

/*
	categories_.resize(3);

	categories_[0].name = "Graphics";
	categories_[1].name = "Physics";
	categories_[2].name = "Hardware";

	categories_[0].items.resize(5);

	categories_[0].items[0].name = "Texture Packs";
	categories_[0].items[0].description = "lorem ipsum dolor sit amet";

	categories_[0].items[1].name = "Jumping Balls";
	categories_[0].items[1].description = "consectetur adipiscing elit";

	categories_[0].items[2].name = "Hamcokelek";
	categories_[0].items[2].description = "this the description text";

	categories_[0].items[3].name = "Texture Packs";
	categories_[0].items[3].description = "lorem ipsum dolor sit amet";

	categories_[0].items[4].name = "Jumping Balls";
	categories_[0].items[4].description = "consectetur adipiscing elit";

//	categories_[0].items[5].name = "Hamcokelek";
//	categories_[0].items[5].description = "this the description text";
*/

	selected_ = -1;

	QPushButton* right = new QPushButton(">", this);
	QPushButton* left = new QPushButton("<", this);

	left->setGeometry(250-60+6, 270+36, 20, 20);
	right->setGeometry(270-60+6, 270+36, 20, 20);

	connect(left, SIGNAL(clicked()), this, SLOT(left()));
	connect(right, SIGNAL(clicked()), this, SLOT(right()));
}

void ExampleProjectsWidget::paintEvent(QPaintEvent* event)
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
//	painter.drawLine(width_/2, 20, width_/2, height_);

	painter.setPen(Qt::black);
	painter.setFont(bold);
	painter.drawText(10, 0, width_, 20, Qt::AlignVCenter, "Example Projects");

	Category& category = categories_[page_];
	std::vector<Item>& items = category.items;

	painter.setFont(title);
	painter.setPen(Qt::black);
	painter.drawText(10, 22, width_, 30, Qt::AlignVCenter, category.name);

	for (size_t i = 0; i < items.size(); ++i)
	{
#if 0
		int y = (i / 2) * 50 + 30;
		int x = (i % 2) * (width_ / 2);

		if (i == selected_)
		{
			painter.setBrush(QBrush(0x3875D7));
			painter.setPen(Qt::NoPen);
			painter.drawRect(x + 1, y + 26, (width_/2)-1, 50);

			painter.setPen(Qt::white);
			painter.setFont(bold);
			painter.drawText(x + 55, y + 28, (width_ / 2)-55, 50, 0, items[i].name);

			painter.setPen(Qt::white);
			painter.setFont(normal);
			painter.drawText(x + 55, y + 44, (width_ / 2)-55, 50, Qt::TextWordWrap, items[i].description);

			painter.setBrush(Qt::black);
			painter.setPen(Qt::black);
			painter.drawRoundedRect(x+10, y + 30, 41, 41, 4, 4);
		}
		else
		{
			painter.setPen(Qt::black);
			painter.setFont(bold);
			painter.drawText(x + 55, y + 28, (width_ / 2)-55, 50, 0, items[i].name);

			painter.setPen(QColor(0x808080));
			painter.setFont(normal);
			painter.drawText(x + 55, y + 44, (width_ / 2)-55, 50, Qt::TextWordWrap, items[i].description);

			painter.setBrush(Qt::black);
			painter.setPen(Qt::black);
			painter.drawRoundedRect(x + 10, y + 30, 41, 41, 4, 4);
		}
#endif
		int y = i * 50 + 26;
		int x = 0;

		if (items[i].image.isNull())
			items[i].image = QImage(items[i].imagePath);

		if (i == selected_)
		{
			painter.setBrush(QBrush(0x3875D7));
			painter.setPen(Qt::NoPen);
			painter.drawRect(x + 1, y + 26, width_ - 1, 50);

			painter.setPen(Qt::white);
			painter.setFont(bold);
			painter.drawText(x + 58, y + 28, width_ - 60, 50, 0, items[i].name);

			painter.setPen(Qt::white);
			painter.setFont(normal);
			painter.drawText(x + 58, y + 44, width_ - 60, 50, Qt::TextWordWrap, items[i].description);

			painter.drawImage(x + 10, y + 29, items[i].image);

/*			painter.setBrush(Qt::white);
			painter.setPen(Qt::black);
			painter.drawRoundedRect(x + 10, y + 30, 41, 41, 4, 4); */
		}
		else
		{
			painter.setPen(Qt::black);
			painter.setFont(bold);
			painter.drawText(x + 58, y + 28, width_ - 60, 50, 0, items[i].name);

			painter.setPen(QColor(0x808080));
			painter.setFont(normal);
			painter.drawText(x + 58, y + 44, width_ - 60, 50, Qt::TextWordWrap, items[i].description);

			painter.drawImage(x + 10, y + 29, items[i].image);

/*			painter.setBrush(Qt::white);
			painter.setPen(Qt::black);
			painter.drawRoundedRect(x + 10, y + 30, 41, 41, 4, 4); */
		}
	}

	QWidget::paintEvent(event);
}

int ExampleProjectsWidget::itemAt(int mx, int my)
{
	const Category& category = categories_[page_];
	const std::vector<Item>& items = category.items;

	for (size_t i = 0; i < items.size(); ++i)
	{
#if 0
		int y = (i / 2) * 50 + 30;
		int x = (i % 2) * (width_ / 2);

		QRect rect(x + 1, y + 26, (width_/2)-1, 50);
#else
		int y = i * 50 + 30;
		int x = 0;

		QRect rect(x + 1, y + 26, width_ - 1, 50);
#endif

		if (rect.contains(mx, my))
			return i;

	}

	return -1;
}

void ExampleProjectsWidget::mousePressEvent(QMouseEvent* event)
{
	selected_ = itemAt(event->x(), event->y());
	update();
	emit selected(selected_);
}

void ExampleProjectsWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
	int item = itemAt(event->x(), event->y());
	if (item != -1)
	{
		const Category& category = categories_[page_];
		emit openProject(category.items[item].path);
	}
}

void ExampleProjectsWidget::left()
{
	int oldpage = page_;
	page_--;
	if (page_ < 0)
		page_ = 0;

	if (oldpage != page_)
	{
		selected_ = -1;
		update();
		emit selected(selected_);
	}
}

void ExampleProjectsWidget::right()
{
	int oldpage = page_;
	page_++;
	if (page_ > categories_.size() - 1)
		page_ = categories_.size() - 1;
	if (oldpage != page_)
	{
		selected_ = -1;
		update();
		emit selected(selected_);
	}
}

void ExampleProjectsWidget::clearSelection()
{
	selected_ = -1;
	update();
}
