#include "librarytreewidget.h"

#include <QAction>
#include <QFileIconProvider>
#include <QMenu>
#include <QFileDialog>
#include <QDir>
#include <stack>
#include <QTimer>
#include <queue>
#include "iconlibrary.h"
#include <QFileInfo>
#include <QMessageBox>
#include <QDir>
#include <QFileInfo>
#include <algorithm>

#include "addnewfiledialog.h"

LibraryTreeWidget::LibraryTreeWidget(QWidget *parent)
	: QTreeWidget(parent)
{
	setSelectionMode(QAbstractItemView::ExtendedSelection);
	setDragEnabled(true);
	viewport()->setAcceptDrops(true);
	setDropIndicatorShown(true);
	setDragDropMode(QAbstractItemView::InternalMove);

	setHeaderHidden(true);

	setEditTriggers(QAbstractItemView::EditKeyPressed | QAbstractItemView::SelectedClicked);

	addNewFileAction_ = new QAction(tr("Add New File..."), this);
	connect(addNewFileAction_, SIGNAL(triggered()), this, SLOT(addNewFile()));

	importToLibraryAction_ = new QAction(tr("Import to Library..."), this);
	connect(importToLibraryAction_, SIGNAL(triggered()), this, SLOT(importToLibrary()));

//	newFontAction_ = new QAction(tr("New Font..."), this);
//	connect(newFontAction_, SIGNAL(triggered()), this, SLOT(newFont()));

	newFolderAction_ = new QAction(tr("New Folder"), this);
	connect(newFolderAction_, SIGNAL(triggered()), this, SLOT(newFolder()));

	removeAction_ = new QAction(tr("Remove"), this);
	connect(removeAction_, SIGNAL(triggered()), this, SLOT(remove()));

	renameAction_ = new QAction(tr("Rename"), this);
	//renameAction_->setShortcut(Qt::Key_F2);
	connect(renameAction_, SIGNAL(triggered()), this, SLOT(rename()));

	sortAction_ = new QAction(tr("Sort"), this);
	connect(sortAction_, SIGNAL(triggered()), this, SLOT(sort()));

	codeDependenciesAction_ = new QAction(tr("Code Dependencies..."), this);
	connect(codeDependenciesAction_, SIGNAL(triggered()), this, SLOT(codeDependencies()));

	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, SIGNAL(customContextMenuRequested  (const QPoint&)),
			this, SLOT  (onCustomContextMenuRequested(const QPoint&)));

	connect(this, SIGNAL(itemDoubleClicked  (QTreeWidgetItem*, int)),
			this, SLOT  (onItemDoubleClicked(QTreeWidgetItem*, int)));

	connect(this, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)),
			this, SLOT  (onCurrentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)));

	isModifed_ = false;
	xmlString_ = toXml().toString();

	QTimer* timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(checkModification()));
	timer->start(500);
}

LibraryTreeWidget::~LibraryTreeWidget()
{

}

void LibraryTreeWidget::onCustomContextMenuRequested(const QPoint& pos)
{
	QMenu menu(this);

	const int file = 1 << 0;
	const int folder = 1 << 1;
	int types = 0;
	int size = selectedItems().size();

	for (int i = 0; i < selectedItems().size(); ++i)
	{
		QString fileName = selectedItems()[i]->data(0, Qt::UserRole).toString();

		if (fileName.isEmpty() == true)
			types |= folder;
		else
			types |= file;
	}

	if (size == 0 || (size == 1 && types == folder))
	{
		menu.addAction(addNewFileAction_);
		menu.addAction(importToLibraryAction_);
		menu.addAction(newFolderAction_);
	}

	if (size > 0)
		menu.addAction(removeAction_);
	if (size == 1 && types == folder)
		menu.addAction(renameAction_);
	if (size == 0 || (size == 1 && types == folder))
		menu.addAction(sortAction_);

	if (size == 1)
	{
		QString fileName = selectedItems()[0]->data(0, Qt::UserRole).toString();

		QFileInfo fileInfo(fileName);

		if (fileInfo.suffix().toLower() == "lua")
			menu.addAction(codeDependenciesAction_);
	}

	menu.exec(QCursor::pos());
}

void LibraryTreeWidget::importToLibrary()
{
	QTreeWidgetItem* root = invisibleRootItem();

	if (selectedItems().empty() == false)
		root = selectedItems().front();

	QString path = QFileInfo(projectFileName_).path();
	QDir dir(path);

	QStringList fileNames = QFileDialog::getOpenFileNames(this, tr("Import to Library"), path, tr("All Files (*.*)"));

	QStringList::Iterator it = fileNames.begin();
	while(it != fileNames.end())
	{
		QString fileName = dir.relativeFilePath(*it);

		if (isFileAlreadyImported(fileName))
		{
			QMessageBox::information(this, tr("Gideros"), tr("The file '%1' cannot be added to the library because it is already a member of the library.").arg(fileName));
		}
		else
		{
			QTreeWidgetItem *item = createFileItem(fileName);
			if (root == invisibleRootItem())
				root->addChild(item);
			else
				root->insertChild(0, item);
			root->setExpanded(true);
		}

		++it;
	}
	checkModification();
}

/*
void LibraryTreeWidget::newFont()
{
	printf("newFont()\n");
}
*/

void LibraryTreeWidget::remove()
{
	QList<QTreeWidgetItem*> selectedItems = this->selectedItems();

	for (int i = 0; i < selectedItems.size(); ++i)
	{
		QTreeWidgetItem* item = selectedItems[i];

		QString fileName = item->data(0, Qt::UserRole).toString();

		QFileInfo fileInfo(fileName);
			
		if (fileInfo.suffix().toLower() == "lua")
			dependencyGraph_.removeCode(fileName);

		delete item;
	}

	checkModification();
}

void LibraryTreeWidget::rename()
{
	if (selectedItems().empty() == true)
		return;

	QTreeWidgetItem* item = selectedItems()[0];

	editItem(item, 0);

	checkModification();
}

void LibraryTreeWidget::newFolder()
{
	QTreeWidgetItem* root = invisibleRootItem();

	if (selectedItems().empty() == false)
		root = selectedItems().front();

	QTreeWidgetItem *item = createFolderItem("New Folder");

	root->setExpanded(true);
	if (root == invisibleRootItem())
		root->addChild(item);
	else
		root->insertChild(0, item);

	editItem(item, 0);

	checkModification();
}

void LibraryTreeWidget::onItemDoubleClicked(QTreeWidgetItem* item, int column)
{
	QString fileName = item->data(0, Qt::UserRole).toString();

	if (fileName.isEmpty() == false)
	{
		QDir dir = QFileInfo(projectFileName_).dir();
		emit openRequest(item->text(0), QDir::cleanPath(dir.absoluteFilePath(fileName)));
	}
}

void LibraryTreeWidget::onCurrentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous)
{
	if (current != 0)
	{
		QString fileName = current->data(0, Qt::UserRole).toString();

		if (fileName.isEmpty() == false)
		{
			QDir dir = QFileInfo(projectFileName_).dir();
			emit previewRequest(current->text(0), QDir::cleanPath(dir.absoluteFilePath(fileName)));
		}
	}
}

QDomDocument LibraryTreeWidget::toXml() const
{
	QDomDocument doc;
	QDomElement root = doc.createElement("library");
	doc.appendChild(root);

	std::stack<std::pair<QTreeWidgetItem*, QDomElement> > stack;
	stack.push(std::make_pair(invisibleRootItem(), root));

	while (stack.empty() == false)
	{
		QTreeWidgetItem* item = stack.top().first;
		QDomElement element = stack.top().second;
		stack.pop();

		for (int i = 0; i < item->childCount(); ++i)
		{
			QTreeWidgetItem* childItem = item->child(i);

			QString fileName = childItem->data(0, Qt::UserRole).toString();

			QDomElement childElement = doc.createElement(fileName.isEmpty() ? "folder" : "file");
			if (fileName.isEmpty() == false)
				childElement.setAttribute("file", fileName);
			else
				childElement.setAttribute("name", childItem->text(0));

			element.appendChild(childElement);

			stack.push(std::make_pair(childItem, childElement));
		}
	}

	std::vector<std::pair<QString, QString> > dependencies = dependencyGraph_.dependencies();
	for (std::size_t i = 0; i < dependencies.size(); ++i)
	{
		QDomElement childElement = doc.createElement("dependency");

		childElement.setAttribute("from", dependencies[i].first);
		childElement.setAttribute("to", dependencies[i].second);

		root.appendChild(childElement);
	}

	return doc;
}

void LibraryTreeWidget::checkModification()
{
	QString xmlString = toXml().toString();
	if (xmlString == xmlString_)
		return;

	bool changed = isModifed_ == false;

	isModifed_ = true;
	xmlString_ = xmlString;

	if (changed)
		emit modificationChanged(isModifed_);
}

void LibraryTreeWidget::clear()
{
	QTreeWidget::clear();
	dependencyGraph_.clear();
	projectFileName_ = "";
	xmlString_ = toXml().toString();

	bool changed = isModifed_ == true;
	isModifed_ = false;

	if (changed)
		emit modificationChanged(isModifed_);
}

void LibraryTreeWidget::loadXml(const QString& projectFileName, const QDomDocument& doc)
{
	dependencyGraph_.clear();

	std::vector<std::pair<QString, QString> > dependencies;

	QTreeWidget::clear();

	std::deque<std::pair<QTreeWidgetItem*, QDomNode> > stack;
	stack.push_back(std::make_pair(static_cast<QTreeWidgetItem*>(0), doc.documentElement()));

	while (stack.empty() == false)
	{
		QTreeWidgetItem* parent = stack.front().first;
		QDomNode n = stack.front().second;
		stack.pop_front();

		QTreeWidgetItem* item = 0;
		if (parent == 0)
		{
			item = invisibleRootItem();
		}
		else
		{
			QDomElement e = n.toElement();

			QString type = e.tagName();

			if (type == "file")
			{
				QString file = e.attribute("file");
				item = createFileItem(file);
			}
			else if (type == "folder")
			{
				QString name = e.attribute("name");
				item = createFolderItem(name);
			}
			else if (type == "dependency")
			{
				QString from = e.attribute("from");
				QString to = e.attribute("to");

				dependencies.push_back(std::make_pair(from, to));
			}

			parent->addChild(item);
		}

		QDomNodeList childNodes = n.childNodes();
		for (int i = 0; i < childNodes.size(); ++i)
			stack.push_back(std::make_pair(item, childNodes.item(i)));
	}

	for (std::size_t i = 0; i < dependencies.size(); ++i)
		dependencyGraph_.addDependency(dependencies[i].first, dependencies[i].second);

	projectFileName_ = projectFileName;

	xmlString_ = toXml().toString();

	bool changed = isModifed_ == true;
	isModifed_ = false;

	if (changed)
		emit modificationChanged(isModifed_);
}

QTreeWidgetItem* LibraryTreeWidget::createFileItem(const QString& file)
{
	QString name = QFileInfo(file).fileName();
	QString ext = QFileInfo(file).suffix().toLower();

	QIcon icon;
	if (ext == "png")
		icon = IconLibrary::instance().icon("picture");
	else if (ext == "lua")
		icon = IconLibrary::instance().icon("lua");
	else if (ext == "mp3" || ext == "wav")
		icon = IconLibrary::instance().icon("sound");
	else
		icon = IconLibrary::instance().icon("file");

	QStringList strings;
	strings << name;
	QTreeWidgetItem *item = new QTreeWidgetItem(strings);
	item->setIcon(0, icon);
	item->setFlags(
		Qt::ItemIsSelectable | 
		Qt::ItemIsDragEnabled |
		Qt::ItemIsEnabled);
	item->setData(0, Qt::UserRole, file);

	if (ext == "lua")
		dependencyGraph_.addCode(file);

	return item;
}

QTreeWidgetItem* LibraryTreeWidget::createFolderItem(const QString& name)
{
	QStringList strings;
	strings << name;
	QTreeWidgetItem *item = new QTreeWidgetItem(strings);
	item->setIcon(0, IconLibrary::instance().icon("folder"));
	item->setFlags(
		Qt::ItemIsSelectable | 
		Qt::ItemIsDragEnabled |
		Qt::ItemIsDropEnabled |
		Qt::ItemIsEnabled |
		Qt::ItemIsEditable);

	item->setData(0, Qt::UserRole, QString());

	return item;
}

void LibraryTreeWidget::setProjectFileName(const QString& fileName)
{
	projectFileName_ = fileName;
}

void LibraryTreeWidget::setModified(bool m)
{
	if (m == false)
		xmlString_ = toXml().toString();

	if (m == isModifed_)
		return;

	isModifed_ = m;

	emit modificationChanged(isModifed_);
}

bool LibraryTreeWidget::isModified() const
{
	return isModifed_;
}

void LibraryTreeWidget::codeDependencies()
{
	QString selected = selectedItems()[0]->data(0, Qt::UserRole).toString();

	CodeDependenciesDialog codeDependencies(&dependencyGraph_, selected, this);

	codeDependencies.exec();
}

bool LibraryTreeWidget::isFileAlreadyImported(const QString& fileName)
{
	std::stack<QTreeWidgetItem*> stack;
	stack.push(invisibleRootItem());

	while (stack.empty() == false)
	{
		QTreeWidgetItem* item = stack.top();
		stack.pop();

		if (fileName == item->data(0, Qt::UserRole).toString())
			return true;

		for (int i = 0; i < item->childCount(); ++i)
			stack.push(item->child(i));
	}

	return false;
}



struct LessQTreeWidgetItem
{
public:
	bool operator()(QTreeWidgetItem* i0, QTreeWidgetItem* i1) const
	{
		int f0 = i0->data(0, Qt::UserRole).toString().isEmpty() ? 0 : 1;		// folders has lower priority (sorts to top)
		int f1 = i1->data(0, Qt::UserRole).toString().isEmpty() ? 0 : 1;

		if (f0 < f1)
			return true;
		if (f0 > f1)
			return false;

		return i0->text(0) < i1->text(0);
	}
};


void LibraryTreeWidget::sort()
{
	QTreeWidgetItem* root = invisibleRootItem();

	if (selectedItems().empty() == false)
		root = selectedItems()[0];

	QList<QTreeWidgetItem*> children = root->takeChildren();

	std::sort(children.begin(), children.end(), LessQTreeWidgetItem());

	root->addChildren(children);
}

void LibraryTreeWidget::addNewFile()
{
	QDir dir = QFileInfo(projectFileName_).dir();
	AddNewFileDialog addNewFile(dir.path(), this);

	while (1)
	{
		if (addNewFile.exec() == QDialog::Accepted)
		{
			QDir newdir = dir;
		
			if (newdir.cd(addNewFile.location()) == false)
			{
				QMessageBox::critical(this, tr("Gideros"), tr("Directory %1 does not exist.").arg(addNewFile.location()));
				continue;
			}

			QString filename = newdir.absoluteFilePath(addNewFile.fileName());

			QFile file(filename);

			// check if it is exists or not
			if (file.exists() == true)
			{
				QMessageBox::critical(this, tr("Gideros"), tr("A file with the name %1 already exists on disk.").arg(filename));
				continue;
			}

			// TODO: check if this file is already on the project. bunu bi dusun. library'deki ismimi yoksa diskteki ismimi onemlidir
			//QString relfilename = dir.relativeFilePath(filename);
			//if (isFileAlreadyImported(relfilename))
			//{
			//	QMessageBox::information(this, tr("Gideros"), tr("The file '%1' cannot be added to the library because it is already a member of the library.").arg(filename));
			//	continue;
			//}

			// try to create an empty file
			if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
			{
				QMessageBox::critical(this, tr("Gideros"), tr("The file %1 could not be created.").arg(filename));
				continue;
			}
			file.close();

			// add file to the project
			QTreeWidgetItem* root = invisibleRootItem();

			if (selectedItems().empty() == false)
				root = selectedItems().front();

			QTreeWidgetItem *item = createFileItem(dir.relativeFilePath(filename));
			if (root == invisibleRootItem())
				root->addChild(item);
			else
				root->insertChild(0, item);
			root->setExpanded(true);

			break;
		}
		else
		{
			break;
		}
	}
}

static QTreeWidgetItem* childWithText(QTreeWidgetItem* item, int column, const QString& text)
{
	for (int i = 0; i < item->childCount(); ++i)
		if (item->child(i)->text(column) == text)
			return item->child(i);

	return 0;
}

QString LibraryTreeWidget::fileName(const QString& itemName) const
{
	QStringList items = itemName.split("/");

	if (items.empty() == true)
		return QString();

	QTreeWidgetItem* item = invisibleRootItem();

	for (int i = 0; i < items.size(); ++i)
	{
		item = childWithText(item, 0, items[i]);
		if (item == 0)
			return QString();
	}

	return item->data(0, Qt::UserRole).toString();
}
