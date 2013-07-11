#include "librarywidget.h"
#include <QMenu>
#include <QContextMenuEvent>
#include <QAction>
#include <QFileIconProvider>

LibraryWidget::LibraryWidget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	connect(ui.treeWidget, SIGNAL(openRequest(const QString&, const QString&)), this, SIGNAL(openRequest(const QString&, const QString&)));
	connect(ui.treeWidget, SIGNAL(previewRequest(const QString&, const QString&)), this, SIGNAL(previewRequest(const QString&, const QString&)));
	connect(ui.treeWidget, SIGNAL(modificationChanged(bool)), this, SIGNAL(modificationChanged(bool)));
}

LibraryWidget::~LibraryWidget()
{

}

QDomDocument LibraryWidget::toXml() const
{
	return ui.treeWidget->toXml();
}


void LibraryWidget::clear()
{
	ui.treeWidget->clear();
}

void LibraryWidget::loadXml(const QString& projectFileName, const QDomDocument& doc)
{
	ui.treeWidget->loadXml(projectFileName, doc);
}

void LibraryWidget::setProjectFileName(const QString& fileName)
{
	ui.treeWidget->setProjectFileName(fileName);
}

void LibraryWidget::setModified(bool m)
{
	ui.treeWidget->setModified(m);
}
bool LibraryWidget::isModified() const
{
	return ui.treeWidget->isModified();
}

QString LibraryWidget::fileName(const QString& itemName) const
{
	return ui.treeWidget->fileName(itemName);
}
