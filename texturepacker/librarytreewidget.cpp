#include "librarytreewidget.h"
#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QFileInfo>
#include <QDir>
#include <QFileDialog>
#include <stack>
#include "iconlibrary.h"
#include <QTimer>
#include <algorithm>

LibraryTreeWidget::LibraryTreeWidget(QWidget *parent)
	: QTreeWidget(parent)
{
	setHeaderHidden(true);

	setSelectionMode(QAbstractItemView::ExtendedSelection);
	setDragEnabled(true);
	viewport()->setAcceptDrops(true);
	setDropIndicatorShown(true);
	setDragDropMode(QAbstractItemView::InternalMove);

	invisibleRootItem()->setFlags(invisibleRootItem()->flags() & ~Qt::ItemIsDropEnabled);

	setEditTriggers(QAbstractItemView::EditKeyPressed | QAbstractItemView::SelectedClicked);

	importToLibraryAction_ = new QAction(tr("Add Existing Files..."), this);
	connect(importToLibraryAction_, SIGNAL(triggered()), this, SLOT(importToLibrary()));

	newFolderAction_ = new QAction(tr("New Folder"), this);
	connect(newFolderAction_, SIGNAL(triggered()), this, SLOT(newFolder()));

	removeAction_ = new QAction(tr("Remove"), this);
	connect(removeAction_, SIGNAL(triggered()), this, SLOT(remove()));

	renameAction_ = new QAction(tr("Rename"), this);
	connect(renameAction_, SIGNAL(triggered()), this, SLOT(rename()));

	sortAction_ = new QAction(tr("Sort"), this);
	connect(sortAction_, SIGNAL(triggered()), this, SLOT(sort()));

	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, SIGNAL(customContextMenuRequested  (const QPoint&)),
			this, SLOT  (onCustomContextMenuRequested(const QPoint&)));

	setWindowModified(false);
	setWindowTitle("[*]");
	xmlString_ = toXml().toString();

	QTimer* timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(checkModification()));
	timer->start(500);
}

LibraryTreeWidget::~LibraryTreeWidget()
{

}

void LibraryTreeWidget::newProject(const QString& projectFileName)
{
	QTreeWidget::clear();

	QTreeWidgetItem* rootitem = createProjectItem(QFileInfo(projectFileName).completeBaseName());
	addTopLevelItem(rootitem);

	projectFileName_ = projectFileName;

	xmlString_ = toXml().toString();

	bool changed = isWindowModified() == true;
	setWindowModified(false);

	if (changed)
		emit modificationChanged(false);
}


void LibraryTreeWidget::importToLibrary()
{
	QTreeWidgetItem* root = invisibleRootItem();

	if (selectedItems().empty() == false)
		root = selectedItems().front();

	QString path = QFileInfo(projectFileName_).path();
	QDir dir(path);

	QStringList fileNames = QFileDialog::getOpenFileNames(this, tr("Import to Library"), path, tr("PNG Files (*.png)"));

	QStringList::Iterator it = fileNames.begin();
	while(it != fileNames.end())
	{
		QString fileName = dir.relativeFilePath(*it);

		if (isFileAlreadyImported(fileName))
		{
			QMessageBox::information(this, tr("Texture Packer"), tr("The file '%1' cannot be added to the library because it is already a member of the library.").arg(fileName));
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

void LibraryTreeWidget::onCustomContextMenuRequested(const QPoint& pos)
{
    Q_UNUSED(pos);
	QMenu menu(this);

	bool folder = false;
	bool project = false;

	int size = selectedItems().size();

	for (int i = 0; i < selectedItems().size(); ++i)
	{
		if (selectedItems()[i]->parent() == NULL)
		{
			project = true;
		}
		else
		{
			QString fileName = selectedItems()[i]->data(0, Qt::UserRole).toString();

			if (fileName.isEmpty() == true)
				folder = true;
		}
	}


	if (size == 1 && (folder || project))
	{
		menu.addAction(importToLibraryAction_);
		menu.addAction(newFolderAction_);
	}

	if (size > 0 && !project)
		menu.addAction(removeAction_);
	if (size == 1 && folder)
		menu.addAction(renameAction_);
	if (size == 1 && (folder || project))
		menu.addAction(sortAction_);

	if (!menu.isEmpty())
		menu.exec(QCursor::pos());
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

	return item;
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

QTreeWidgetItem* LibraryTreeWidget::createFolderItem(const QString& name)
{
	QStringList strings;
	strings << name;
	QTreeWidgetItem *item = new QTreeWidgetItem(strings);
	item->setIcon(0, IconLibrary::instance().icon("folder"));		// TODO
	item->setFlags(
		Qt::ItemIsSelectable | 
		Qt::ItemIsDragEnabled |
		Qt::ItemIsDropEnabled |
		Qt::ItemIsEnabled |
		Qt::ItemIsEditable);

	item->setData(0, Qt::UserRole, QString());

	return item;
}

QTreeWidgetItem* LibraryTreeWidget::createProjectItem(const QString& name)
{
	QStringList strings;
	strings << name;
	QTreeWidgetItem *item = new QTreeWidgetItem(strings);
	QFont font = item->font(0);
	font.setBold(true);
	item->setFont(0, font);
	item->setIcon(0, IconLibrary::instance().icon("project"));
	item->setFlags(
		Qt::ItemIsSelectable |
		Qt::ItemIsDropEnabled |
		Qt::ItemIsEnabled);

	item->setData(0, Qt::UserRole, QString());

	return item;
}

QDomDocument LibraryTreeWidget::toXml() const
{
	QDomDocument doc;
	QDomElement root = doc.createElement("library");

	QDomElement properties = doc.createElement("properties");
    properties.setAttribute("padding", properties_.padding);
    properties.setAttribute("extrude", properties_.extrude);
    properties.setAttribute("forceSquare", properties_.forceSquare ? 1 : 0);
	properties.setAttribute("removeAlphaBorder", properties_.removeAlphaBorder ? 1 : 0);
	properties.setAttribute("alphaThreshold", properties_.alphaThreshold);
	properties.setAttribute("showUnusedAreas", properties_.showUnusedAreas ? 1 : 0);

	root.appendChild(properties);

	doc.appendChild(root);

	QTreeWidgetItem* rootitem = invisibleRootItem();
	if (rootitem->childCount())
		rootitem = rootitem->child(0);

	std::stack<std::pair<QTreeWidgetItem*, QDomElement> > stack;
	stack.push(std::make_pair(rootitem, root));

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

	return doc;
}

void LibraryTreeWidget::checkModification()
{
	QString xmlString = toXml().toString();
	if (xmlString == xmlString_)
		return;

	xmlString_ = xmlString;

	emit changed();

	bool changed = isWindowModified() == false;
	setWindowModified(true);

	if (changed)
		emit modificationChanged(true);
}

void LibraryTreeWidget::clear()
{
	QTreeWidget::clear();
	projectFileName_ = "";
	properties_.clear();
	xmlString_ = toXml().toString();

	bool changed = isWindowModified() == true;
	setWindowModified(false);

	if (changed)
		emit modificationChanged(false);
}


void LibraryTreeWidget::setWindowModified(bool m)
{
	bool current = isWindowModified();
	QTreeWidget::setWindowModified(m);

	if (current != m)
		emit modificationChanged(m);
}


void LibraryTreeWidget::loadXml(const QString& projectFileName, const QDomDocument& doc)
{
	QTreeWidget::clear();


	QDomElement root = doc.documentElement();

	// read properties
	{
		properties_.clear();

		QDomElement properties = root.firstChildElement("properties");

        if (!properties.attribute("padding").isEmpty())
            properties_.padding = properties.attribute("padding").toInt();
        if (!properties.attribute("extrude").isEmpty())
            properties_.extrude = properties.attribute("extrude").toInt();
        if (!properties.attribute("forceSquare").isEmpty())
			properties_.forceSquare = properties.attribute("forceSquare").toInt() != 0;
		if (!properties.attribute("removeAlphaBorder").isEmpty())
			properties_.removeAlphaBorder = properties.attribute("removeAlphaBorder").toInt() != 0;
		if (!properties.attribute("alphaThreshold").isEmpty())
			properties_.alphaThreshold = properties.attribute("alphaThreshold").toDouble();
		if (!properties.attribute("showUnusedAreas").isEmpty())
			properties_.showUnusedAreas = properties.attribute("showUnusedAreas").toInt() != 0;
	}


	QTreeWidgetItem* rootitem = createProjectItem(QFileInfo(projectFileName).completeBaseName());
	addTopLevelItem(rootitem);

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
			item = rootitem;//invisibleRootItem();
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

			parent->addChild(item);
		}

		QDomNodeList childNodes = n.childNodes();
		for (int i = 0; i < childNodes.size(); ++i)
			stack.push_back(std::make_pair(item, childNodes.item(i)));
	}

	rootitem->setExpanded(true);
	
	projectFileName_ = projectFileName;

	xmlString_ = toXml().toString();

	bool changed = isWindowModified() == true;
	setWindowModified(false);

	if (changed)
		emit modificationChanged(false);
}

void LibraryTreeWidget::remove()
{
	QList<QTreeWidgetItem*> selectedItems = this->selectedItems();

	for (int i = 0; i < selectedItems.size(); ++i)
	{
		QTreeWidgetItem* item = selectedItems[i];
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

ProjectProperties& LibraryTreeWidget::getProjectProperties()
{
	return properties_;
}


