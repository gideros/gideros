#include "mdiarea.h"
#include <QVBoxLayout>
#include <QTabBar>
#include <QTabBar>
#include "mdisubwindow.h"
#include <QDebug>
#include <QAction>
#include "textedit.h"

MyMdiArea::MyMdiArea(QWidget *parent) :
    QWidget(parent)
{
	layout_ = new QVBoxLayout(this);
	layout_->setSpacing(0);
	layout_->setContentsMargins(0, 0, 0, 0);

	tabBar_ = new QTabBar(this);

	connect(tabBar_, SIGNAL(currentChanged(int)), this, SLOT(onCurrentChanged(int)));
	connect(tabBar_, SIGNAL(tabMoved(int, int)), this, SLOT(onTabMoved(int, int)));

	layout_->addWidget(tabBar_);

	emptyWidget_ = new QWidget(this);
	layout_->addWidget(emptyWidget_);
	currentWidget_ = emptyWidget_;
}

MyMdiArea::~MyMdiArea()
{

}

QList<QString> MyMdiArea::tabFilenameOrderList()
{
    if (recentTabSelectionOrder_.size() != subWindows_.size())
    {
        QList<QString> nameList;
        for (int nn = 0; nn < subWindows_.size(); ++nn)
        {
            TextEdit* textEdit = dynamic_cast<TextEdit*>(subWindows_[nn]);
            if (textEdit != NULL)
            {
                QString filename = textEdit->fileName();
                nameList.append(filename);
            }
        }

        for (int n = 0; n < recentTabSelectionOrder_.size(); ++n)
        {
            QString name = recentTabSelectionOrder_[n];
            bool found = false;
            for (int nn = 0; nn < nameList.size(); ++nn)
            {
                if (name == nameList[nn])
                {
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                recentTabSelectionOrder_.removeAt(n);
                --n;
            }
        }
    }
    return recentTabSelectionOrder_;
}

int MyMdiArea::getCurrentTabIndex()
{
    for (int i = 0; i < subWindows_.size(); ++i)
    {
        if (currentWidget_ == subWindows_[i])
        {
            return i;
        }
    }
    return -1;
}

void MyMdiArea::changeCurrentTabIndex(int index)
{
    if (index >= 0 && index < subWindows_.size())
    {
        tabBar_->setCurrentIndex(index);
    }
}

void MyMdiArea::changeCurrentTabByFilename(QString filenameToChange)
{
    for (int i = 0; i < subWindows_.size(); ++i)
    {
        TextEdit* textEdit = dynamic_cast<TextEdit*>(subWindows_[i]);
        if (textEdit != NULL)
        {
            QString filename = textEdit->fileName();
            if (filename == filenameToChange)
            {
                changeCurrentTabIndex(i);
                return;
            }
        }
    }
}

void MyMdiArea::addSubWindow(MyMdiSubWindow* window)
{
	Q_ASSERT(subWindows_.contains(window) == false);

	currentWidget_->hide();
	layout_->removeWidget(currentWidget_);

	layout_->addWidget(window);
	window->show();
	currentWidget_ = window;

	bool blocked = tabBar_->blockSignals(true);
	tabBar_->addTab(tabText(window));
	tabBar_->setCurrentIndex(tabBar_->count() - 1);
	tabBar_->blockSignals(blocked);

	QWidget* left = tabBar_->tabButton(tabBar_->count() - 1, QTabBar::LeftSide);
	QWidget* right = tabBar_->tabButton(tabBar_->count() - 1, QTabBar::RightSide);

	subWindows_.append(window);

	connect(window, SIGNAL(windowTitleChanged(const QString&)), this, SLOT(onWindowModified()));
	connect(window, SIGNAL(modifiedChanged(bool)), this, SLOT(onWindowModified()));
	connect(window, SIGNAL(destroyed(QObject*)), this, SLOT(onDestroyed(QObject*)));

    addToOrderList(window);

	emit subWindowActivated(activeSubWindow());
}

void MyMdiArea::onCurrentChanged(int index)
{
	currentWidget_->hide();
	layout_->removeWidget(currentWidget_);

	currentWidget_ = subWindows_[index];
	layout_->addWidget(currentWidget_);
	currentWidget_->show();

    setTopOrderList(currentWidget_);

	emit subWindowActivated(activeSubWindow());


    currentWidget_->setFocus();

    TextEdit* textEdit = dynamic_cast<TextEdit*>(currentWidget_);
    if (textEdit != NULL)
    {
        textEdit->setFocusToEdit();
    }
}

void MyMdiArea::onTabMoved(int from, int to)
{
	MyMdiSubWindow* window = subWindows_[from];
	subWindows_.removeAt(from);
	subWindows_.insert(to, window);
}

void MyMdiArea::setActiveSubWindow(MyMdiSubWindow* window)
{
	int index = subWindows_.indexOf(window);
	Q_ASSERT(index >= 0);
	tabBar_->setCurrentIndex(index);
}

MyMdiSubWindow* MyMdiArea::activeSubWindow() const
{
	return currentWidget_ == emptyWidget_ ? NULL : static_cast<MyMdiSubWindow*>(currentWidget_);
}

QList<MyMdiSubWindow*> MyMdiArea::subWindowList() const
{
	return subWindows_;
}

void MyMdiArea::closeActiveSubWindow()
{
	Q_ASSERT(tabBar_->count() > 0);

	int index = tabBar_->currentIndex();
	MyMdiSubWindow* window = subWindows_[index];

    removeFromOrderList(window);

	if (window->close())
		removeFromTab(window);
}

void MyMdiArea::removeFromTab(MyMdiSubWindow* window)
{
	int index = subWindows_.indexOf(window);

	if (index >= 0)
	{
		subWindows_.removeAt(index);

		bool blocked = tabBar_->blockSignals(true);
		tabBar_->removeTab(index);
		tabBar_->blockSignals(blocked);

		layout_->removeWidget(window);

        removeFromOrderList(window);

		currentWidget_ = (tabBar_->count() == 0) ? emptyWidget_ : subWindows_[tabBar_->currentIndex()];
		layout_->addWidget(currentWidget_);
		currentWidget_->show();

        setTopOrderList(currentWidget_);

		emit subWindowActivated(activeSubWindow());
	}
}

void MyMdiArea::onDestroyed(QObject* object)
{
	MyMdiSubWindow* window = static_cast<MyMdiSubWindow*>(object);
	removeFromTab(window);
}


QString MyMdiArea::tabText(MyMdiSubWindow* window) const
{
	QString result = window->windowTitle();

	if (window->isWindowModified())
		result.replace("[*]", QChar(0x2022));
	else
		result.replace("[*]", "");

	return result;
}

void MyMdiArea::updateTabText(MyMdiSubWindow* window)
{
	int index = subWindows_.indexOf(window);
	if (index >= 0)
		tabBar_->setTabText(index, tabText(window));
}

void MyMdiArea::onWindowModified()
{
	updateTabText(static_cast<MyMdiSubWindow*>(sender()));
}

void MyMdiArea::addToOrderList(QWidget* window)
{
    TextEdit* textEdit = dynamic_cast<TextEdit*>(window);
    if (textEdit != NULL)
    {
        QString filename = textEdit->fileName();
        recentTabSelectionOrder_.insert(0, filename);
    }
}

void MyMdiArea::removeFromOrderList(QWidget* window)
{
    TextEdit* textEdit = dynamic_cast<TextEdit*>(window);
    if (textEdit != NULL)
    {
        QString filename = textEdit->fileName();
        for (int i = 0; i < recentTabSelectionOrder_.size(); ++i)
        {
            if (recentTabSelectionOrder_[i] == filename)
            {
                recentTabSelectionOrder_.removeAt(i);
                break;
            }
        }
    }
}

void MyMdiArea::setTopOrderList(QWidget* window)
{
    TextEdit* textEdit = dynamic_cast<TextEdit*>(window);
    if (textEdit != NULL)
    {
        QString filename = textEdit->fileName();
        for (int i = 0; i < recentTabSelectionOrder_.size(); ++i)
        {
            if (recentTabSelectionOrder_[i] == filename)
            {
                recentTabSelectionOrder_.removeAt(i);
                recentTabSelectionOrder_.insert(0, filename);
                break;
            }
        }
    }
}
