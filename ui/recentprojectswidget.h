#ifndef RECENTPROJECTSWIDGET_H
#define RECENTPROJECTSWIDGET_H

#include <QWidget>

class RecentProjectsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit RecentProjectsWidget(QWidget *parent = 0);

signals:
	void selected(int);
	void openProject(const QString&);

public slots:
	void clearSelection();

protected:
	virtual void paintEvent(QPaintEvent *);
	virtual void mousePressEvent(QMouseEvent* event);
	virtual void mouseDoubleClickEvent(QMouseEvent* event);

private:
	int itemAt(int x, int y);

	struct Item
	{
		QString name;
		QString path;
	};

	std::vector<Item> items_;
	int selected_;
	int width_, height_;
};

#endif // RECENTPROJECTSWIDGET_H
