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
	connect(ui.treeWidget, SIGNAL(modificationChanged(bool)), this, SLOT(onModificationChanged(bool)));
	connect(ui.treeWidget, SIGNAL(insertIntoDocument(const QString&)), this, SIGNAL(insertIntoDocument(const QString&)));
	connect(ui.treeWidget, SIGNAL(automaticDownsizingEnabled(const QString&)), this, SIGNAL(automaticDownsizingEnabled(const QString&)));
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

void LibraryWidget::newProject(const QString& projectFileName)
{
	ui.treeWidget->newProject(projectFileName);
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

void LibraryWidget::onModificationChanged(bool m)
{
	ui.label->setText(tr("Project") + (m ? "*" : ""));
	emit modificationChanged(m);
}
