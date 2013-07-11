#ifndef MDISUBWINDOW_H
#define MDISUBWINDOW_H

#include <QWidget>

class QVBoxLayout;

class MyMdiSubWindow : public QWidget
{
    Q_OBJECT
public:
	explicit MyMdiSubWindow(QWidget *parent = 0);

	void setWidget(QWidget *widget);
	QWidget *widget() const;

signals:
	void windowTitleChanged(const QString& title);
	void modifiedChanged(bool modified);

private:
	QVBoxLayout* layout_;
	QWidget* widget_;
	QAction* closeAction_;
};

class MyEventFilter : public QObject
{
	Q_OBJECT
public:
	MyEventFilter(QObject* parent = 0);

signals:
	void windowTitleChanged(const QString& title);
	void modifiedChanged(bool modified);

protected:
	bool eventFilter(QObject *obj, QEvent *event);
};



//#include <QMdiSubWindow>
//#define MdiSubWindow QMdiSubWindow

#define MdiSubWindow MyMdiSubWindow

#endif // MDISUBWINDOW_H
