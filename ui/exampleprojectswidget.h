#ifndef EXAMPLEPROJECTSWIDGET_H
#define EXAMPLEPROJECTSWIDGET_H

#include <QWidget>
#include <QImage>

class ExampleProjectsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ExampleProjectsWidget(QWidget *parent = 0);

signals:
	void selected(int);
	void openProject(const QString&);

public slots:
	void clearSelection();

private slots:
	void left();
	void right();

protected:
	virtual void paintEvent(QPaintEvent *);
	virtual void mousePressEvent(QMouseEvent* event);
	virtual void mouseDoubleClickEvent(QMouseEvent* event);

private:
	int itemAt(int mx, int my);

	struct Item
	{
		QString name;
		QString description;
		QString path;
		QImage image;
		QString imagePath;
	};

	struct Category
	{
		QString name;
		std::vector<Item> items;
	};

	std::vector<Category> categories_;

	static int page_;
	int selected_;

	int width_, height_;
};

#endif // EXAMPLEPROJECTSWIDGET_H
