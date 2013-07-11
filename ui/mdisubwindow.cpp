#include "mdisubwindow.h"
#include <QVBoxLayout>
#include <QEvent>
#include <QDebug>
#include <QAction>

MyMdiSubWindow::MyMdiSubWindow(QWidget *parent) :
    QWidget(parent)
{
	widget_ = NULL;

	layout_ = new QVBoxLayout(this);
	layout_->setSpacing(0);
	layout_->setContentsMargins(0, 0, 0, 0);

	MyEventFilter* eventFilter = new MyEventFilter(this);
	connect(eventFilter, SIGNAL(windowTitleChanged(const QString&)), this, SIGNAL(windowTitleChanged(const QString&)));
	connect(eventFilter, SIGNAL(modifiedChanged(bool)), this, SIGNAL(modifiedChanged(bool)));
	installEventFilter(eventFilter);

	closeAction_ = new QAction("Close", this);
	closeAction_->setShortcut( QKeySequence("Ctrl+W"));
	connect(closeAction_, SIGNAL(triggered()), this, SLOT(close()));
	addAction(closeAction_);
}

void MyMdiSubWindow::setWidget(QWidget* widget)
{
	Q_ASSERT(widget_ == NULL);
	layout_->addWidget(widget);
	widget_ = widget;
}

QWidget* MyMdiSubWindow::widget() const
{
	return widget_;
}

MyEventFilter::MyEventFilter(QObject* parent) :
	QObject(parent)
{

}

bool MyEventFilter::eventFilter(QObject* obj, QEvent* event)
{
	if (event->type() == QEvent::WindowTitleChange)
	{
		QWidget* window = qobject_cast<QWidget*>(obj);
		if (window)
			emit windowTitleChanged(window->windowTitle());
	}
	else if (event->type() == QEvent::ModifiedChange)
	{
		QWidget* window = qobject_cast<QWidget*>(obj);
		if (window)
			emit modifiedChanged(window->isWindowModified());
	}

	return QObject::eventFilter(obj, event);
}
