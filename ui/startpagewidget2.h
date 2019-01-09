#ifndef STARTPAGEWIDGET2_H
#define STARTPAGEWIDGET2_H

#include <QWidget>
#include <QMdiSubWindow>
#include "mdisubwindow.h"

namespace Ui {
    class StartPageWidget2;
}

class StartPageWidget2 : public MdiSubWindow
{
    Q_OBJECT

public:
    explicit StartPageWidget2(QWidget *parent = 0);
    ~StartPageWidget2();

signals:
	void openProject(const QString&);
	void newProject();

private slots:
	void referenceManual();

private:
    Ui::StartPageWidget2 *ui;

protected:
	virtual void closeEvent(QCloseEvent * event);
};

#endif // STARTPAGEWIDGET2_H
