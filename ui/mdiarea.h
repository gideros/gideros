#ifndef MDIAREA_H
#define MDIAREA_H

#include <QWidget>

class MyMdiSubWindow;
class QTabBar;
class QVBoxLayout;

class MyMdiArea : public QWidget
{
    Q_OBJECT
public:
	explicit MyMdiArea(QWidget *parent = 0);
	virtual ~MyMdiArea();

	void addSubWindow(MyMdiSubWindow* widget);
	MyMdiSubWindow* activeSubWindow() const;
	QList<MyMdiSubWindow*> subWindowList() const;

    QList<QString> tabFilenameOrderList();

    int getCurrentTabIndex();
    void changeCurrentTabIndex(int index);
    void changeCurrentTabByFilename(QString filename);

signals:
	void subWindowActivated(MyMdiSubWindow* window);

public slots:
	void setActiveSubWindow(MyMdiSubWindow* window);
	void closeActiveSubWindow();

private slots:
	void onCurrentChanged(int);
	void onTabMoved(int, int);
	void onWindowModified();
	void onDestroyed(QObject*);

private:
	QString tabText(MyMdiSubWindow* window) const;
	void updateTabText(MyMdiSubWindow* window);
	void removeFromTab(MyMdiSubWindow* window);

    void addToOrderList(QWidget* window);
    void removeFromOrderList(QWidget* window);
    void setTopOrderList(QWidget* window);

private:
	QVBoxLayout* layout_;
	QTabBar* tabBar_;
	QWidget* emptyWidget_;
	QWidget* currentWidget_;

    QList<QString> recentTabSelectionOrder_;

	QList<MyMdiSubWindow*> subWindows_;
};

//#include <QMdiArea>
//#define MdiArea QMdiArea

#define MdiArea MyMdiArea


#endif // MDIAREA_H
