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
#include <QDirIterator>
#include <QFileInfo>
#include <QDebug>
#include <algorithm>

#include "addnewfiledialog.h"
#include "projectpropertiesdialog.h"
#include "pluginselector.h"
#include "plugineditor.h"
#include "qtutils.h"
#include "addons.h"

#define NODETYPE_PROJECT	1
#define NODETYPE_FILE		2
#define NODETYPE_FOLDER		4
#define NODETYPE_PLUGINS	8
#define NODETYPE_FILES		16
#define NODETYPE_PLUGIN		32

LibraryTreeWidget *LibraryTreeWidget::lua_instance=nullptr;

static QTreeWidgetItem *tw_getFilesRoot(lua_State *L,int index)
{
	if (!LibraryTreeWidget::lua_instance)
        return nullptr;
	QTreeWidgetItem* rootitem = LibraryTreeWidget::lua_instance->invisibleRootItem();
    if (!rootitem) return nullptr;
	rootitem = rootitem->child(0);
    if (!rootitem) return nullptr;
	QTreeWidgetItem *filesFolder=rootitem->child(1);
    if (lua_type(L,index)==LUA_TTABLE) {
        size_t tl=lua_objlen(L,index);
        for (size_t k=1;k<=tl;k++)
        {
            lua_rawgeti(L,index,(int)k);
            const char *ename=luaL_checkstring(L,-1);
            lua_pop(L,1);
            QTreeWidgetItem *sub=nullptr;
            for (int i = 0; i < filesFolder->childCount(); ++i)
            {
                QTreeWidgetItem* childItem = filesFolder->child(i);
                QString name=childItem->text(0);
                if (name==QString(ename))
                    sub=childItem;
            }
            if (!sub)
            {
                lua_pushstring(L,"Couldn't locate path");
                lua_error(L);
            }
            else
                filesFolder=sub;
        }
    }
	return filesFolder;
}

static void tw_dumpFiles(lua_State *L,QTreeWidgetItem *item);

static void tw_dumpItem(lua_State *L,QTreeWidgetItem *childItem) {
    QMap<QString, QVariant> data = childItem->data(0, Qt::UserRole).toMap();
    QString fileName = data["filename"].toString();
    QString name=childItem->text(0);
    lua_newtable(L);
    lua_pushstring(L,name.toUtf8().constData());
    lua_setfield(L,-2,"name");
    if (fileName.isEmpty()) {  //Dir
        tw_dumpFiles(L,childItem);
        lua_setfield(L,-2,"folder");
    }
    else
    {
        QDir dir = QFileInfo(LibraryTreeWidget::lua_instance->projectFileName_).dir();
        lua_pushstring(L,QDir::cleanPath(dir.absoluteFilePath(fileName)).toUtf8().constData());
        lua_setfield(L,-2,"source");
#define BOOL_ATTRIB(n) 	if (data.contains(n)) { lua_pushboolean(L, data[n].toBool()); lua_setfield(L,-2,n); }
        BOOL_ATTRIB("downsizing");
        BOOL_ATTRIB("excludeFromExecution");
        BOOL_ATTRIB("excludeFromEncryption");
        BOOL_ATTRIB("excludeFromPackage");
#undef BOOL_ATTRIB
    }
}

static void tw_dumpFiles(lua_State *L,QTreeWidgetItem *item) {
	lua_newtable(L);
	for (int i = 0; i < item->childCount(); ++i)
	{
		QTreeWidgetItem* childItem = item->child(i);
        tw_dumpItem(L,childItem);
		lua_rawseti(L,-2,i+1);
	}
}

static int ltw_listFiles(lua_State *L) {
    QTreeWidgetItem *files=tw_getFilesRoot(L,1);
	if (!files) {
		lua_pushnil(L);
		return 1;
	}
	tw_dumpFiles(L,files);
	return 1;
}

static int ltw_addFile(lua_State *L) {
    QTreeWidgetItem *files=tw_getFilesRoot(L,1);
    if (!files) {
        lua_pushnil(L);
		return 1;
	}
    const char *fname=luaL_checkstring(L,2);
    QMap<QString, QVariant> data;
    if (!lua_isnoneornil(L,3))
    {
        luaL_checktype(L,3,LUA_TTABLE);
#define GETFIELD(n) lua_getfield(L,3,n); data[n]=lua_toboolean(L,-1); lua_pop(L,1);
        GETFIELD("downsizing");
        GETFIELD("excludeFromExecution");
        GETFIELD("excludeFromEncryption");
        GETFIELD("excludeFromPackage");
#undef GETFIELD
    }

    QTreeWidgetItem *n=LibraryTreeWidget::lua_instance->newFile(files,QString(fname),data);
    if (n)
        tw_dumpItem(L,n);
    else
        lua_pushnil(L);

    return 1;
}

int ltw_removeFile(lua_State *L) {
    QTreeWidgetItem *files=tw_getFilesRoot(L,1);
    if (!files) {
        lua_pushboolean(L,false);
		return 1;
	}
    LibraryTreeWidget::lua_instance->remove(files);
    lua_pushboolean(L,true);
	return 1;
}

int ltw_addFolder(lua_State *L) {
    QTreeWidgetItem *files=tw_getFilesRoot(L,1);
    if (!files) {
        lua_pushboolean(L,false);
		return 1;
	}
    const char *fname=luaL_checkstring(L,2);
    LibraryTreeWidget::lua_instance->newFolder(files,QString(fname));
    lua_pushboolean(L,true);
    return 1;
}


LibraryTreeWidget::LibraryTreeWidget(QWidget *parent)
	: QTreeWidget(parent)
{
	setSelectionMode(QAbstractItemView::ExtendedSelection);
	setDragEnabled(true);
	viewport()->setAcceptDrops(true);
	setDropIndicatorShown(true);
	setDragDropMode(QAbstractItemView::InternalMove);

	invisibleRootItem()->setFlags(invisibleRootItem()->flags() & ~Qt::ItemIsDropEnabled);

	setHeaderHidden(true);

	setEditTriggers(QAbstractItemView::EditKeyPressed | QAbstractItemView::SelectedClicked);

	addNewFileAction_ = new QAction(tr("Add New File..."), this);
	connect(addNewFileAction_, SIGNAL(triggered()), this, SLOT(addNewFile()));

    importToLibraryAction_ = new QAction(tr("Link Existing Files..."), this);
	connect(importToLibraryAction_, SIGNAL(triggered()), this, SLOT(importToLibrary()));

    importFolderAction_ = new QAction(tr("Link folder..."), this);
	connect(importFolderAction_, SIGNAL(triggered()), this, SLOT(importFolder()));

//	newFontAction_ = new QAction(tr("New Font..."), this);
//	connect(newFontAction_, SIGNAL(triggered()), this, SLOT(newFont()));

	newFolderAction_ = new QAction(tr("New Folder"), this);
	connect(newFolderAction_, SIGNAL(triggered()), this, SLOT(newFolder()));

	removeAction_ = new QAction(tr("Remove"), this);
	connect(removeAction_, SIGNAL(triggered()), this, SLOT(remove()));

	renameAction_ = new QAction(tr("Rename"), this);
	//renameAction_->setShortcut(Qt::Key_F2);
	connect(renameAction_, SIGNAL(triggered()), this, SLOT(rename()));

    refreshAction_ = new QAction(tr("Refresh"), this);
    refreshAction_->setShortcut(Qt::Key_F3);
    connect(refreshAction_, SIGNAL(triggered()), this, SLOT(refresh()));

	sortAction_ = new QAction(tr("Sort"), this);
	connect(sortAction_, SIGNAL(triggered()), this, SLOT(sort()));

	codeDependenciesAction_ = new QAction(tr("Code Dependencies..."), this);
	connect(codeDependenciesAction_, SIGNAL(triggered()), this, SLOT(codeDependencies()));

	insertIntoDocumentAction_ = new QAction(tr("Insert Into Document"), this);
	connect(insertIntoDocumentAction_, SIGNAL(triggered()), this, SLOT(insertIntoDocument()));

	projectPropertiesAction_ = new QAction(tr("Properties..."), this);
	connect(projectPropertiesAction_, SIGNAL(triggered()), this, SLOT(projectProperties()));

	automaticDownsizingAction_ = new QAction(tr("Automatic Downsizing"), this);
	automaticDownsizingAction_->setCheckable(true);
	connect(automaticDownsizingAction_, SIGNAL(triggered(bool)), this, SLOT(automaticDownsizing(bool)));

    excludeFromExecutionAction_ = new QAction(tr("Exclude from Execution"), this);
    excludeFromExecutionAction_->setCheckable(true);
    connect(excludeFromExecutionAction_, SIGNAL(triggered(bool)), this, SLOT(excludeFromExecution(bool)));

    excludeFromEncryptionAction_ = new QAction(tr("Never encrypt"), this);
    excludeFromEncryptionAction_->setCheckable(true);
    connect(excludeFromEncryptionAction_, SIGNAL(triggered(bool)), this, SLOT(excludeFromEncryption(bool)));

    excludeFromPackageAction_ = new QAction(tr("Don't package"), this);
    excludeFromPackageAction_->setCheckable(true);
    connect(excludeFromPackageAction_, SIGNAL(triggered(bool)), this, SLOT(excludeFromPackage(bool)));

#if defined(Q_OS_WIN)
    showInFindeAction_ = new QAction(tr("Show in Explorer"), this);
#else
    showInFindeAction_ = new QAction(tr("Show in Finder"), this);
#endif
	connect(showInFindeAction_, SIGNAL(triggered()), this, SLOT(showInFinder()));

	addPluginAction_ = new QAction(tr("Add plugin"), this);
	connect(addPluginAction_, SIGNAL(triggered()), this, SLOT(addPlugin()));
	propPluginAction_ = new QAction(tr("Properties..."), this);
	connect(propPluginAction_, SIGNAL(triggered()), this, SLOT(propPlugin()));

    setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, SIGNAL(customContextMenuRequested  (const QPoint&)),
			this, SLOT  (onCustomContextMenuRequested(const QPoint&)));

	connect(this, SIGNAL(itemDoubleClicked  (QTreeWidgetItem*, int)),
			this, SLOT  (onItemDoubleClicked(QTreeWidgetItem*, int)));

    connect(this, SIGNAL(itemChanged  (QTreeWidgetItem*, int)),
            this, SLOT  (onItemChanged(QTreeWidgetItem*, int)));

	connect(this, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)),
			this, SLOT  (onCurrentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)));

	isModifed_ = false;
    xmlString_ = toXml();

	QTimer* timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(checkModification()));
	timer->start(500);

	lua_State *L=AddonsManager::getLua();
    luaL_Reg reg[] = {
        { "listFiles", ltw_listFiles },
        { "addFile", ltw_addFile },
        { "removeFile", ltw_removeFile },
        { "addFolder", ltw_addFolder },
        { nullptr, nullptr }
    };
    luaL_register(L,"Studio",reg);
    lua_pop(L,1);
    lua_instance=this;
}

LibraryTreeWidget::~LibraryTreeWidget()
{
 if (this==lua_instance) lua_instance=nullptr;
}

void LibraryTreeWidget::onCustomContextMenuRequested(const QPoint& /*pos*/)
{
	QMenu menu(this);

	int nodetype=0;

	int size = selectedItems().size();

	for (int i = 0; i < selectedItems().size(); ++i)
	{
		QMap<QString, QVariant> data=selectedItems()[i]->data(0, Qt::UserRole).toMap();
		nodetype |=data ["nodetype"].toInt();
	}

    //add "show in finder" in the first position just as Xcode did
    if (size == 1 && ((nodetype&NODETYPE_PROJECT) || (nodetype&NODETYPE_FILE)))
        menu.addAction(showInFindeAction_);

    if (size == 1 && (nodetype&NODETYPE_PLUGINS))
        menu.addAction(addPluginAction_);

    if (size == 1 && ((nodetype&NODETYPE_FOLDER) || (nodetype&NODETYPE_FILES)))
	{
		menu.addAction(addNewFileAction_);
		menu.addAction(importToLibraryAction_);
		menu.addAction(newFolderAction_);
		menu.addAction(importFolderAction_);
        menu.addAction(refreshAction_);
    }

	if (size > 0 && (nodetype&(NODETYPE_FILE|NODETYPE_FOLDER|NODETYPE_PLUGIN)))
		menu.addAction(removeAction_);
	if (size == 1 && (nodetype&NODETYPE_FOLDER))
        menu.addAction(renameAction_);
	if (size == 1 && ((nodetype&NODETYPE_FOLDER) || (nodetype&NODETYPE_FILES)))
		menu.addAction(sortAction_);

	if (size == 1 && (nodetype&NODETYPE_FILE))
	{
		menu.addAction(insertIntoDocumentAction_);

		QMap<QString, QVariant> data = selectedItems()[0]->data(0, Qt::UserRole).toMap();

		QString fileName = data["filename"].toString();

		QFileInfo fileInfo(fileName);

		QString ext = fileInfo.suffix().toLower();

		if (ext == "lua")
        {
            menu.addAction(codeDependenciesAction_);
            bool excludeFromExecution = data.contains("excludeFromExecution") && data["excludeFromExecution"].toBool();
            excludeFromExecutionAction_->setChecked(excludeFromExecution);
            menu.addAction(excludeFromExecutionAction_);
        }

		if (ext == "png" || ext == "jpg" || ext == "jpeg")
		{
			bool downsizing = data.contains("downsizing") && data["downsizing"].toBool();
			automaticDownsizingAction_->setChecked(downsizing);
			menu.addAction(automaticDownsizingAction_);
		}

        bool excludeFromEncryption = data.contains("excludeFromEncryption") && data["excludeFromEncryption"].toBool();
        excludeFromEncryptionAction_->setChecked(excludeFromEncryption);
        bool excludeFromPackage = data.contains("excludeFromPackage") && data["excludeFromPackage"].toBool();
        excludeFromPackageAction_->setChecked(excludeFromPackage);
        menu.addAction(excludeFromEncryptionAction_);
        menu.addAction(excludeFromPackageAction_);
	}

	if (size == 1 && (nodetype&NODETYPE_PROJECT))
		menu.addAction(projectPropertiesAction_);
	if (size == 1 && (nodetype&NODETYPE_PLUGIN))
		menu.addAction(propPluginAction_);

	if (!menu.isEmpty())
		menu.exec(QCursor::pos());
}

bool LibraryTreeWidget::hasItemNamed(QTreeWidgetItem* root,QString name, bool link)
{
	for (int i = 0; i < root->childCount(); ++i)
        if ((!name.compare(root->child(i)->text(0)))&&(root->data(0, Qt::UserRole).toMap()["link"].toBool()==link))
			return true;
	return false;
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

        if (isFileAlreadyImported(fileName,true))
		{
			QMessageBox::information(this, tr("Gideros"), tr("The file '%1' cannot be added to the library because it is already a member of the library.").arg(fileName));
		}
		else
		{
			QString name=QFileInfo(fileName).fileName();
            if (hasItemNamed(root,name,true))
				QMessageBox::information(this, tr("Gideros"), tr("The file '%1' cannot be added here because there is already a file named '%2' in this folder.").arg(fileName).arg(name));
			else
			{
				QTreeWidgetItem *item = createFileItem(fileName,true);
				if (root == invisibleRootItem())
					root->addChild(item);
				else
					root->insertChild(0, item);
				root->setExpanded(true);
			}
		}

		++it;
	}
	checkModification();
}

void LibraryTreeWidget::importFolder()
{
	QTreeWidgetItem* root = invisibleRootItem();

	if (selectedItems().empty() == false)
		root = selectedItems().front();

	QString path = QFileInfo(projectFileName_).path();
	QDir dir(path);

	QString basedir = QFileDialog::getExistingDirectory(this, tr("Import folder content"), path);
	if(basedir.isEmpty())
		return;

    bool recursiveImport=false;

    if (recursiveImport)
    {
        struct _Folder {
            QString basedir;
            QTreeWidgetItem *root;
            _Folder(QString d,QTreeWidgetItem *i) : basedir(d), root(i) { }
        };

        std::vector<_Folder> stack;
        stack.push_back(_Folder(basedir,root));

        while (stack.size()>0)
        {
            _Folder f=stack.back();
            stack.pop_back();
            basedir=f.basedir;
            root=f.root;
            QFileInfoList entryList=QDir(basedir).entryInfoList( QStringList() << "*", QDir::Dirs|QDir::Files|QDir::NoSymLinks|QDir::NoDotAndDotDot|QDir::Readable );

            QFileInfoList::Iterator it = entryList.begin();
            while(it != entryList.end())
            {
                if ((*it).isFile())
                {
                    QString fileName = dir.relativeFilePath((*it).absoluteFilePath());
                    QString name=QFileInfo(fileName).fileName();

                    if (isFileAlreadyImported(fileName,true)||hasItemNamed(root,name,true))
                    {
                        //QMessageBox::information(this, tr("Gideros"), tr("The file '%1' cannot be added to the library because it is already a member of the library.").arg(fileName));
                    }
                    else
                    {
                        QTreeWidgetItem *item = createFileItem(fileName,true);
                        if (root == invisibleRootItem())
                            root->addChild(item);
                        else
                            root->insertChild(0, item);
                        root->setExpanded(true);
                    }
                }
                else if ((*it).isDir())
                {
                    QDir dir = QFileInfo(projectFileName_).dir();
                    QString path=getItemPath(root)+"/"+(*it).fileName();
                    if (!dir.cd(path))
                        if (!dir.mkpath(path))
                        {
                            //QMessageBox::critical(this, tr("Gideros"), tr("Directory %1 couldn't be created.").arg(path));
                        }

                    QTreeWidgetItem *item = createFolderItem((*it).fileName(),QString());

                    root->setExpanded(true);
                    if (root == invisibleRootItem())
                        root->addChild(item);
                    else
                        root->insertChild(0, item);

                    stack.push_back(_Folder((*it).filePath(),item));
                }

                ++it;
            }
        }
    }
    else
    {
        //Just link
        basedir=dir.relativeFilePath(basedir);
        QFileInfo baseInfo(basedir);
        QTreeWidgetItem *item = createFolderItem(baseInfo.fileName(),basedir);

        root->setExpanded(true);
        if (root == invisibleRootItem())
            root->addChild(item);
        else
            root->insertChild(0, item);

        refreshFolder(item);
    }
    checkModification();
}

/*
void LibraryTreeWidget::newFont()
{
	printf("newFont()\n");
}
*/


void LibraryTreeWidget::showInFinder()
{
    if (selectedItems().empty() == true)
        return;

    QTreeWidgetItem* item = selectedItems()[0];


    QString path;

    if (item->parent() == nullptr){
        path = projectFileName_;
    }else
    {
        QString fileName = item->data(0, Qt::UserRole).toMap()["filename"].toString();

        QDir dir = QFileInfo(projectFileName_).dir();

        path = QDir::cleanPath(dir.absoluteFilePath(fileName));
    }

    doShowInFinder(path);
}

void LibraryTreeWidget::addPlugin()
{
    if (selectedItems().empty() == true)
        return;
    QTreeWidgetItem* pluginFolder = selectedItems()[0];
    PluginSelector dialog(properties_.plugins, this);
	if (dialog.exec() == QDialog::Accepted)
	{
		QString plugin=dialog.selection();
		if (!plugin.isEmpty())
		{
			ProjectProperties::Plugin m;
			bool found=false;
			for (QSet<ProjectProperties::Plugin>::iterator it=properties_.plugins.begin();it!=properties_.plugins.end(); it++)
			{
				if ((*it).name==plugin)
				{
					m=*it;
					properties_.plugins.erase(it);
					found=true;
					break;
				}
			}
			if (!found)
				m.name=plugin;
			m.enabled=true;
			properties_.plugins.insert(m);
			pluginFolder->addChild(createPluginItem(plugin));
		}
	}
}

void LibraryTreeWidget::propPlugin()
{
    if (selectedItems().empty() == true)
        return;
    QMap<QString, QVariant> data=selectedItems()[0]->data(0, Qt::UserRole).toMap();
	int nodetype = data ["nodetype"].toInt();
	QString fileName = data["filename"].toString();

	if (nodetype==NODETYPE_PLUGIN) {

		ProjectProperties::Plugin m;
		bool found=false;
		for (QSet<ProjectProperties::Plugin>::iterator it=properties_.plugins.begin();it!=properties_.plugins.end(); it++)
		{
			if ((*it).name==selectedItems()[0]->text(0))
			{
				m=*it;
				properties_.plugins.erase(it);
				found=true;
				break;
			}
		}
		if (found)
		{
            PluginEditor dialog(&m,QFileInfo(projectFileName_).dir(), this);
			dialog.exec();
			properties_.plugins.insert(m);
		}
	}
}

void LibraryTreeWidget::remove()
{
	QList<QTreeWidgetItem*> selectedItems = this->selectedItems();

	for (int i = 0; i < selectedItems.size(); ++i)
	{
		QTreeWidgetItem* item = selectedItems[i];
        remove(item);
	}
}

void LibraryTreeWidget::refresh()
{
    if (selectedItems().empty() == true)
        return;

    QTreeWidgetItem* item = selectedItems()[0];
    refreshFolder(item);
}

void LibraryTreeWidget::rename()
{
	if (selectedItems().empty() == true)
		return;

	QTreeWidgetItem* item = selectedItems()[0];

	editItem(item, 0);

	checkModification();
}

QTreeWidgetItem* LibraryTreeWidget::filesRootFolder()
{
    QTreeWidgetItem* rootitem = invisibleRootItem();
    rootitem = rootitem->child(0);
    if (!rootitem) return nullptr;
    return rootitem->child(1);
}

void LibraryTreeWidget::newFolder()
{
    QTreeWidgetItem* root = filesRootFolder();

	if (selectedItems().empty() == false)
		root = selectedItems().front();

    if ((!root)||hasItemNamed(root,"New Folder",false)) return;
    QDir dir = QFileInfo(projectFileName_).dir();
    QString path=getItemPath(root)+"/New Folder";
    if (!dir.cd(path))
        if (!dir.mkpath(path))
        {
            QMessageBox::critical(this, tr("Gideros"), tr("Directory %1 couldn't be created.").arg(path));
            return;
        }

    QTreeWidgetItem *item = createFolderItem("New Folder",QString());

	root->setExpanded(true);
    root->addChild(item);
    sortFolder(root);

	editItem(item, 0);

	checkModification();
}

QTreeWidgetItem *LibraryTreeWidget::newFile(QTreeWidgetItem *parent,QString name, QMap<QString, QVariant> data)
{
    QDir dir = QFileInfo(projectFileName_).dir();
    QString filename = dir.absoluteFilePath(name);
    if (!data["link"].toBool()) {
        // check if it is exists or not
        QFile file(filename);
        if (file.exists() == true) return nullptr; //File exists
                // try to create an empty file
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return nullptr; // Can't create
        file.close();
    }

    QTreeWidgetItem *item = createFileItem(dir.relativeFilePath(filename),data["link"].toBool(),
           data["downsizing"].toBool(),data["excludeFromExecution"].toBool(),data["excludeFromEncryption"].toBool(),data["excludeFromPackage"].toBool());
    parent->addChild(item);
    checkModification();
    return item;
}

void LibraryTreeWidget::newFolder(QTreeWidgetItem *parent,QString name)
{
    if (hasItemNamed(parent,name,false)) return;
    QDir dir = QFileInfo(projectFileName_).dir();
    QString path=getItemPath(parent)+"/"+name;
    if (!dir.cd(path))
        if (!dir.mkpath(path))
        {
            QMessageBox::critical(this, tr("Gideros"), tr("Directory %1 couldn't be created.").arg(path));
            return;
        }

    QTreeWidgetItem *item = createFolderItem(name,QString());
    parent->insertChild(0, item);
    checkModification();
}

bool LibraryTreeWidget::folderHasLinks(QTreeWidgetItem *item)
{
    std::stack<QTreeWidgetItem*> stack;
    stack.push(item);

    while (!stack.empty())
    {
        QTreeWidgetItem* item = stack.top();
        stack.pop();

        QMap<QString, QVariant> data=item->data(0, Qt::UserRole).toMap();
        QString fspath = data["fspath"].toString();
        if (!fspath.isEmpty()) return true;
        if (data["link"].toBool()) return true;
        for (int i = 0; i < item->childCount(); ++i)
                stack.push(item->child(i));
    }
    return false;
}

void LibraryTreeWidget::refreshFolder(QTreeWidgetItem *item)
{
    std::stack<QTreeWidgetItem*> stack;
    stack.push(item);

    while (!stack.empty())
    {
        QTreeWidgetItem* item = stack.top();
        stack.pop();

        QMap<QString, QVariant> data=item->data(0, Qt::UserRole).toMap();
        int nodetype = data ["nodetype"].toInt();
        QString fspath = data["fspath"].toString();

        if ((nodetype==NODETYPE_FILES)||(nodetype==NODETYPE_FOLDER)) {
        	if (fspath.isEmpty()) {
                //Virtual folder: construct real folder from ancestors
                fspath=getItemPath(item);
        	}

        	// Check if path exists
            QDir dir = QFileInfo(projectFileName_).dir();
            if (!(fspath.isEmpty())) {
                dir.mkdir(fspath);
            }
            if ((!fspath.isEmpty())&&dir.cd(fspath)) {
                // EMPTY LIST and MARK EXISTING
                QMap<QString,QTreeWidgetItem*> map;
                for (int i = item->childCount()-1; i >=0 ; i--)
                {
                	QTreeWidgetItem* sub = item->child(i);
                    QMap<QString, QVariant> sdata=sub->data(0, Qt::UserRole).toMap();
                    if (!(sdata["link"].toBool()||(!sdata["fspath"].toString().isEmpty())))
                    {
                        map[sub->text(0)]=sub;
                        item->removeChild(sub);
                    }
                }
                // SYNC
                QFileInfoList ls=dir.entryInfoList(QStringList(),QDir::Dirs|QDir::Files|QDir::NoDotAndDotDot|QDir::Readable,QDir::DirsFirst|QDir::Name);
                foreach (QFileInfo fi,ls) {
                    if (map.contains(fi.fileName())) {
                        QTreeWidgetItem *sub=map[fi.fileName()];
                        QMap<QString, QVariant> sdata=sub->data(0, Qt::UserRole).toMap();
                        int nodetype = sdata ["nodetype"].toInt();
                        if ((fi.isDir()&&(nodetype==NODETYPE_FOLDER))||((fi.isFile()&&(nodetype==NODETYPE_FILE)))) {
                            item->addChild(sub);
                            if (fi.isFile()) {
                                QString oldName=sdata["filename"].toString();
                                QFileInfo fileInfo(oldName);
                                sdata["filename"]=fspath+"/"+fi.fileName(); //Update file path
                                sub->setData(0, Qt::UserRole, sdata);
                                if (fileInfo.suffix().toLower() == "lua") {
                                    if (dependencyGraph_.hasCode(oldName))
                                        dependencyGraph_.renameCode(oldName,sdata["filename"].toString());
                                    else
                                        dependencyGraph_.addCode(sdata["filename"].toString(),sdata["excludeFromExecution"].toBool());
                                }
                            }
                        }
                        else {
                            if (fi.isDir())
                                sub=createFolderItem(fi.fileName(),QString());
                            else
                                sub=createFileItem(fspath+"/"+fi.fileName(),false);
                            item->addChild(sub);
                        }
                        map.remove(fi.fileName());
                    }
                    else {
                        QTreeWidgetItem *sub=nullptr;
                        if (fi.isDir())
                            sub=createFolderItem(fi.fileName(),QString());
                        else
                            sub=createFileItem(fspath+"/"+fi.fileName(),false);
                        item->addChild(sub);
                    }
             	}
                //ADD BACK REMAINING FOLDER WITH LINKS
                foreach (QTreeWidgetItem *sub,map) {
                    QMap<QString, QVariant> sdata=sub->data(0, Qt::UserRole).toMap();
                    QFileInfo fileInfo(sdata["filename"].toString());

                    if (fileInfo.suffix().toLower() == "lua")
                        dependencyGraph_.removeCode(sdata["filename"].toString());
                    if (folderHasLinks(sub))
                        item->addChild(sub);
                }
                // SORT
                sortFolder(item);
            	// RECURSE
                for (int i = 0; i < item->childCount(); ++i)
                {
                	QTreeWidgetItem* sub = item->child(i);
                    int stype = (sub->data(0, Qt::UserRole).toMap()) ["nodetype"].toInt();
                    if (stype==NODETYPE_FOLDER)
                    	stack.push(sub);
                }
            }
        }
    }
    checkModification();
}

void LibraryTreeWidget::remove(QTreeWidgetItem *item)
{    
    QMap<QString, QVariant> data=item->data(0, Qt::UserRole).toMap();
    int nodetype = data ["nodetype"].toInt();
    QString name=item->text(0);

    QMessageBox::StandardButton reply;
    if (nodetype==NODETYPE_FOLDER)
    {
        QString fspath=getItemPath(item);
        QDir dir = QFileInfo(projectFileName_).dir();
        if (data["fspath"].toString().isEmpty()&&(!fspath.isEmpty()&&dir.cd(fspath))) //Not a link but has a path: this is a real folder
        {
              reply = QMessageBox::question(this, "Delete folder", "Do you really want to delete folder '"+name+"' and all its subfolders ? This cannot be undone.",
                                            QMessageBox::Yes|QMessageBox::Cancel);
              if (reply == QMessageBox::Cancel)
                  return;
              if (!dir.removeRecursively())
                return;
        }
    }
    else if (nodetype==NODETYPE_FILE)
    {
        if (!data["link"].toBool())
        {
            QString fspath=getItemPath(item->parent());
            QDir dir = QFileInfo(projectFileName_).dir();
            if (!fspath.isEmpty()&&dir.cd(fspath))
            {
                reply = QMessageBox::question(this, "Delete file", "Do you really want to delete file '"+name+"' ? This cannot be undone.",
                                              QMessageBox::Yes|QMessageBox::Cancel);
                if (reply == QMessageBox::Cancel)
                    return;
                if (!dir.remove(name))
                    return;
            }
        }
    }

    if (item->parent())
        item->parent()->removeChild(item);

    std::stack<QTreeWidgetItem*> stack;
    stack.push(item);

    while (!stack.empty())
    {
        QTreeWidgetItem* item = stack.top();
        stack.pop();

        QMap<QString, QVariant> data=item->data(0, Qt::UserRole).toMap();
        int nodetype = data ["nodetype"].toInt();
        QString fileName = data["filename"].toString();

        if (nodetype==NODETYPE_PLUGIN) {

            ProjectProperties::Plugin m;
            bool found=false;
            for (QSet<ProjectProperties::Plugin>::iterator it=properties_.plugins.begin();it!=properties_.plugins.end(); it++)
            {
                if ((*it).name==item->text(0))
                {
                    m=*it;
                    properties_.plugins.erase(it);
                    found=true;
                    break;
                }
            }
            if (found)
            {
                m.enabled=false;
                properties_.plugins.insert(m);
            }
        }

        QFileInfo fileInfo(fileName);

        if (fileInfo.suffix().toLower() == "lua")
            dependencyGraph_.removeCode(fileName);

        for (int i = 0; i < item->childCount(); ++i)
            stack.push(item->child(i));
    }

    delete item;

    checkModification();
}

void LibraryTreeWidget::onItemChanged(QTreeWidgetItem* item, int /*column*/)
{
    QString name=item->text(0);
    QMap<QString, QVariant> data=item->data(0, Qt::UserRole).toMap();
    int nodetype = data ["nodetype"].toInt();
    QString oldName = data["oldName"].toString();

    if (nodetype==NODETYPE_FOLDER)
    {
        QString fspath=getItemPath(item->parent());
        QDir dir = QFileInfo(projectFileName_).dir();
        if (!fspath.isEmpty()&&(!oldName.isEmpty())&&dir.cd(fspath))
        {
            //Rename folder
            if (!dir.rename(oldName,name))
                name=oldName;
        }
    }
    else if (nodetype==NODETYPE_FILE)
    {
        if (!data["link"].toBool())
        {
            QString fspath=getItemPath(item->parent());
            QDir dir = QFileInfo(projectFileName_).dir();
            if (!fspath.isEmpty()&&(!oldName.isEmpty())&&dir.cd(fspath))
            {
                //Rename file
                if (!dir.rename(oldName,name))
                    name=oldName;
                else {
                    QFileInfo fileInfo(data["filename"].toString());
                    if (fileInfo.suffix().toLower() == "lua")
                        dependencyGraph_.removeCode(data["filename"].toString());
                    data["filename"]=fspath+"/"+name; //Update file path
                    item->setData(0, Qt::UserRole, data);
                    if (fileInfo.suffix().toLower() == "lua")
                        dependencyGraph_.addCode(data["filename"].toString(),data["excludeFromExecution"].toBool());
                }
            }
        }
    }
    data["oldName"]=name;
    item->setText(0,name);
    sortFolder(item->parent());
}


void LibraryTreeWidget::onItemDoubleClicked(QTreeWidgetItem* item, int /*column*/)
{
	QMap<QString, QVariant> data=item->data(0, Qt::UserRole).toMap();
	int nodetype = data ["nodetype"].toInt();
	QString fileName = data["filename"].toString();

	if (nodetype==NODETYPE_PLUGIN)
		propPlugin();
	else if (fileName.isEmpty() == false)
	{
		QDir dir = QFileInfo(projectFileName_).dir();
		emit openRequest(item->text(0), QDir::cleanPath(dir.absoluteFilePath(fileName)));
	}
}

void LibraryTreeWidget::onCurrentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* /*previous*/)
{
    if (current != nullptr)
	{
		QString fileName = current->data(0, Qt::UserRole).toMap()["filename"].toString();

		if (fileName.isEmpty() == false)
		{
			QDir dir = QFileInfo(projectFileName_).dir();
			emit previewRequest(current->text(0), QDir::cleanPath(dir.absoluteFilePath(fileName)));
		}
	}
}

QString LibraryTreeWidget::toXml() const {
    QString t;
    QXmlStreamWriter out(&t);
    out.setAutoFormatting(true);
    out.writeStartDocument();
    toXml(out);
    out.writeEndDocument();
    return t;
}
/*
 * XML file/folder nodes
 * 	- File Link: <file source="absolute target file" ...attributes...>
 * 	- File Real: <file file="folder relative file name" ...attributes...>
 * 	- Folder Link: <folder name="folder name" path="absolute target folder path">
 * 	- Folder Virtual: <folder name="folder name" path="">
 */
void LibraryTreeWidget::toXml(QXmlStreamWriter &out) const
{
    out.writeStartElement("project");

    out.writeStartElement("properties");
    properties_.toXml(out);
    out.writeEndElement();

    QTreeWidgetItem* rootitem = invisibleRootItem();
    rootitem = rootitem->child(0);
    QTreeWidgetItem *pluginsFolder=rootitem?rootitem->child(0):NULL;
    QTreeWidgetItem *filesFolder=rootitem?rootitem->child(1):NULL;
    if (pluginsFolder&&filesFolder) {

        std::stack<std::pair<QTreeWidgetItem*, bool> > stack;
        stack.push(std::make_pair(filesFolder, true));

        QDir pdir = QFileInfo(projectFileName_).dir();

        while (stack.empty() == false)
        {
            QTreeWidgetItem* item = stack.top().first;
            bool enter = stack.top().second;
            stack.pop();

            if (enter)
            {
                if (item!=filesFolder)
                {
                    stack.push(std::make_pair(item, false));
                    QMap<QString, QVariant> data = item->data(0, Qt::UserRole).toMap();
                    out.writeStartElement("folder");
                    out.writeAttribute("name", item->text(0));
                    if (!data["fspath"].toString().isEmpty())
                        out.writeAttribute("path", pdir.relativeFilePath(data["fspath"].toString()));
                }
                for (int i = 0; i < item->childCount(); ++i)
                {
                    QTreeWidgetItem* childItem = item->child(i);

                    QMap<QString, QVariant> data = childItem->data(0, Qt::UserRole).toMap();
                    QString fileName = data["filename"].toString();

                    if (fileName.isEmpty() == false)
                    {
                        out.writeStartElement("file");
                        bool isLink=data["link"].toBool();
                        out.writeAttribute(isLink?"source":"name",pdir.relativeFilePath(fileName));
                        if (data.contains("downsizing") && data["downsizing"].toBool())
                            out.writeAttribute("downsizing", "1");
                        if (data.contains("excludeFromExecution") && data["excludeFromExecution"].toBool())
                            out.writeAttribute("excludeFromExecution", "1");
                        if (data.contains("excludeFromEncryption") && data["excludeFromEncryption"].toBool())
                            out.writeAttribute("excludeFromEncryption", "1");
                        if (data.contains("excludeFromPackage") && data["excludeFromPackage"].toBool())
                            out.writeAttribute("excludeFromPackage", "1");
                        out.writeEndElement();
                    }
                    else
                        stack.push(std::make_pair(childItem, true));
                }
            }
            else
                out.writeEndElement();
        }

        std::vector<std::pair<QString, QString> > dependencies = dependencyGraph_.dependencies();
        for (std::size_t i = 0; i < dependencies.size(); ++i)
        {
            out.writeStartElement("dependency");
            out.writeAttribute("from",  pdir.relativeFilePath(dependencies[i].first));
            out.writeAttribute("to",  pdir.relativeFilePath(dependencies[i].second));
            out.writeEndElement();
        }
    }

    out.writeEndElement(); //project
}

void LibraryTreeWidget::checkModification()
{
    QString xmlString = toXml();
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
	properties_.clear();
    xmlString_ = toXml();

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

	QDomElement root = doc.documentElement();

	// read properties
	{
		properties_.clear();

		QDomElement properties = root.firstChildElement("properties");
		properties_.loadXml(properties);
    }

	QTreeWidgetItem* rootitem = createProjectItem(QFileInfo(projectFileName).completeBaseName());
	addTopLevelItem(rootitem);
	QTreeWidgetItem* pluginsFolder = createCatFolderItem("Plugins",QString(),"folder plugins",NODETYPE_PLUGINS);
	rootitem->addChild(pluginsFolder);
	QTreeWidgetItem* filesFolder = createCatFolderItem("Files","assets","folder files",NODETYPE_FILES,true);
	rootitem->addChild(filesFolder);

	//Fill in plugins
    QList<ProjectProperties::Plugin> pl=properties_.plugins.values();
    std::sort(pl.begin(),pl.end());
	for (QList<ProjectProperties::Plugin>::const_iterator it=pl.begin();it!=pl.end(); it++)
	{
		ProjectProperties::Plugin p=*it;
		if (p.enabled)
		{
			QTreeWidgetItem* plugin = createPluginItem(p.name);
			pluginsFolder->addChild(plugin );
		}
	}
	//Fill in files
	std::deque<std::pair<QTreeWidgetItem*, QDomNode> > stack;
    stack.push_back(std::make_pair(static_cast<QTreeWidgetItem*>(nullptr), doc.documentElement()));

	while (stack.empty() == false)
	{
		QTreeWidgetItem* parent = stack.front().first;
		QDomNode n = stack.front().second;
		stack.pop_front();

        QTreeWidgetItem* item = nullptr;
        if (parent == nullptr)
		{
            item = filesFolder;
		}
		else
		{
			QDomElement e = n.toElement();

			QString type = e.tagName();

			if (type == "file")
			{
                QString file = e.hasAttribute("name")? e.attribute("name"):(e.hasAttribute("source") ? e.attribute("source") : e.attribute("file"));
                bool downsizing = e.hasAttribute("downsizing") && e.attribute("downsizing").toInt();
                bool excludeFromExecution = e.hasAttribute("excludeFromExecution") && e.attribute("excludeFromExecution").toInt();
                bool excludeFromEncryption = e.hasAttribute("excludeFromEncryption") && e.attribute("excludeFromEncryption").toInt();
                bool excludeFromPackage = e.hasAttribute("excludeFromPackage") && e.attribute("excludeFromPackage").toInt();
                item = createFileItem(file, !e.hasAttribute("name"),downsizing, excludeFromExecution, excludeFromEncryption,excludeFromPackage);
			}
			else if (type == "folder")
			{
				QString name = e.attribute("name");
				item = createFolderItem(name,e.attribute("path"));
			}
			else if (type == "dependency")
			{
				QString from = e.attribute("from");
				QString to = e.attribute("to");

				dependencies.push_back(std::make_pair(from, to));
			}

			if (item)
				parent->addChild(item);
		}

		QDomNodeList childNodes = n.childNodes();
		for (int i = 0; i < childNodes.size(); ++i)
			stack.push_back(std::make_pair(item, childNodes.item(i)));
	}

    projectFileName_ = projectFileName;

	for (std::size_t i = 0; i < dependencies.size(); ++i)
		dependencyGraph_.addDependency(dependencies[i].first, dependencies[i].second);


    xmlString_ = toXml();

	bool changed = isModifed_ == true;
	isModifed_ = false;

	if (changed)
		emit modificationChanged(isModifed_);

    refreshFolder(filesFolder);

    rootitem->setExpanded(true);
    filesFolder->setExpanded(true);

}

QTreeWidgetItem* LibraryTreeWidget::createFileItem(const QString& file, bool link, bool downsizing, bool excludeFromExecution, bool excludeFromEncryption, bool excludeFromPackage)
{
	QString name = QFileInfo(file).fileName();
	QString ext = QFileInfo(file).suffix().toLower();
    QString baseIcon="file";
    QStringList tags;

	if (ext == "png" || ext == "jpg" || ext == "jpeg")
        baseIcon="picture";
	else if (ext == "lua")
    {
        baseIcon="lua";
        if (excludeFromExecution) tags << "stop";
    }
    else if (ext == "mp3" || ext == "wav" || ext == "ogg" || ext == "oga")
        baseIcon="sound";

    if (link)
        tags << "link";
    QIcon icon = IconLibrary::instance().icon(baseIcon,tags);

	QStringList strings;
	strings << name;
	QTreeWidgetItem *item = new QTreeWidgetItem(strings);
	item->setIcon(0, icon);
	item->setFlags(
		Qt::ItemIsSelectable | 
		Qt::ItemIsDragEnabled |
		Qt::ItemIsEnabled);

	QMap<QString, QVariant> data;

	data["filename"] = file;
    data["oldName"] = name;
	data["link"] = link;
	data["nodetype"] = NODETYPE_FILE;

    if (downsizing)
        data["downsizing"] = true;

    if (excludeFromExecution)
        data["excludeFromExecution"] = true;

    if (excludeFromEncryption)
        data["excludeFromEncryption"] = true;

    if (excludeFromPackage)
        data["excludeFromPackage"] = true;

    item->setData(0, Qt::UserRole, data);

	if (ext == "lua")
        dependencyGraph_.addCode(file, excludeFromExecution);

	return item;
}

QTreeWidgetItem* LibraryTreeWidget::createPluginItem(const QString& name)
{
	QIcon icon = IconLibrary::instance().icon(0, "plugin");

	QStringList strings;
	strings << name;
	QTreeWidgetItem *item = new QTreeWidgetItem(strings);
	item->setIcon(0, icon);
	item->setFlags(
		Qt::ItemIsSelectable |
		Qt::ItemIsEnabled);

	QMap<QString, QVariant> data;

	data["filename"] = QString();
	data["nodetype"] = NODETYPE_PLUGIN;
	item->setData(0, Qt::UserRole, data);
	return item;
}

QTreeWidgetItem* LibraryTreeWidget::createFolderItem(const QString& name, const QString& fspath)
{
	QStringList strings;
	strings << name;
    QStringList tags;
    if (!fspath.isEmpty()) tags<<"link";
	QTreeWidgetItem *item = new QTreeWidgetItem(strings);
    item->setIcon(0, IconLibrary::instance().icon("folder",tags));
	item->setFlags(
		Qt::ItemIsSelectable | 
		Qt::ItemIsDragEnabled |
		Qt::ItemIsDropEnabled |
		Qt::ItemIsEnabled |
		Qt::ItemIsEditable);

	QMap<QString, QVariant> data;
	data["filename"] = QString();
	data["fspath"] = fspath;
	data["nodetype"] = NODETYPE_FOLDER;
    data["oldName"] = name;

	item->setData(0, Qt::UserRole, data);

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
	item->setIcon(0, IconLibrary::instance().icon(0, "project"));
	item->setFlags(
		Qt::ItemIsSelectable |
		Qt::ItemIsEnabled);

	QMap<QString, QVariant> data;
	data["filename"] = QString();
	data["nodetype"] = NODETYPE_PROJECT;

	item->setData(0, Qt::UserRole, data);

	return item;
}

QTreeWidgetItem* LibraryTreeWidget::createCatFolderItem(const QString& name,const QString& fspath, const QString& icon, int nodetype, bool drop)
{
	QStringList strings;
	strings << name;
	QTreeWidgetItem *item = new QTreeWidgetItem(strings);
	QFont font = item->font(0);
	font.setBold(true);
	item->setFont(0, font);
	item->setIcon(0, IconLibrary::instance().icon(0, icon));
	item->setFlags(
		Qt::ItemIsSelectable |
		(drop?Qt::ItemIsDropEnabled:(Qt::ItemFlag)0) |
		Qt::ItemIsEnabled);

	QMap<QString, QVariant> data;
	data["filename"] = QString();
	data["fspath"]=fspath;
	data["nodetype"] = nodetype;

	item->setData(0, Qt::UserRole, data);

	return item;
}

void LibraryTreeWidget::newProject(const QString& projectFileName)
{
	dependencyGraph_.clear();

	QTreeWidget::clear();

	properties_.clear();

	QTreeWidgetItem* rootitem = createProjectItem(QFileInfo(projectFileName).completeBaseName());
	addTopLevelItem(rootitem);
	QTreeWidgetItem* pluginsFolder = createCatFolderItem("Plugins",QString(),"folder plugins", NODETYPE_PLUGINS);
	rootitem->addChild(pluginsFolder);
	QTreeWidgetItem* filesFolder = createCatFolderItem("Files","assets","folder files",NODETYPE_FILES, true);
	rootitem->addChild(filesFolder);

	rootitem->setExpanded(true);
	filesFolder->setExpanded(true);

	projectFileName_ = projectFileName;

    xmlString_ = toXml();

	bool changed = isModifed_ == true;
	isModifed_ = false;

	if (changed)
		emit modificationChanged(isModifed_);

    refreshFolder(filesFolder);
}

void LibraryTreeWidget::cloneProject(const QString& projectFileName)
{
    //Copy project assets to new location, keeping links as-is
    std::stack<QTreeWidgetItem*> stack;
    stack.push(filesRootFolder());

    while (!stack.empty())
    {
        QTreeWidgetItem* item = stack.top();
        stack.pop();

        QMap<QString, QVariant> data=item->data(0, Qt::UserRole).toMap();
        int nodetype = data ["nodetype"].toInt();
        QString fspath = data["fspath"].toString();

        if ((nodetype==NODETYPE_FILES)||(nodetype==NODETYPE_FOLDER)) {
            bool linkFolder=(nodetype!=NODETYPE_FILES);
            bool virtualFolder=false;
            if (fspath.isEmpty()) {
                //Virtual folder: construct real folder from ancestors
                virtualFolder=true;
                fspath=getItemPath(item,&linkFolder);
            }

            // Ensure if path exists
            QDir dir = QFileInfo(projectFileName).dir();
            if (!(fspath.isEmpty())) {
                dir.mkdir(fspath);
            }
            if ((!fspath.isEmpty())&&dir.cd(fspath)) {
               QDir pdir = QFileInfo(projectFileName_).dir();
               QDir tdir = QFileInfo(projectFileName).dir();
               for (int i = item->childCount()-1; i >=0 ; i--)
                {
                    QTreeWidgetItem* sub = item->child(i);
                    QMap<QString, QVariant> sdata=sub->data(0, Qt::UserRole).toMap();
                    int stype = sdata["nodetype"].toInt();
                    if (stype==NODETYPE_FOLDER)
                        stack.push(sub);
                    else if (stype==NODETYPE_FILE) {
                        if (sdata["link"].toBool()||linkFolder) //External file, either direct link or through parent folder link
                        {
                            QString oldPath=sdata["filename"].toString();
                            QString newPath=tdir.relativeFilePath(pdir.absoluteFilePath(oldPath));
                            sdata["filename"]=newPath;
                            sub->setData(0, Qt::UserRole, sdata);
                            if (dependencyGraph_.hasCode(oldPath))
                                dependencyGraph_.renameCode(oldPath,newPath);
                        }
                        else {
                            //Copy file to new location
                            QString path=getItemPath(sub);
                            QString oldPath=pdir.absoluteFilePath(path);
                            QString newPath=tdir.absoluteFilePath(path);
                            if (!QFile::copy(oldPath, newPath))
                            {
                                QMessageBox::critical(this, tr("Gideros"), tr("File %1 can't be copied to %2. Cancelling, but some files may have been copied already.").arg(oldPath).arg(newPath));
                                checkModification();
                                return;
                            }
                         }
                    }
                }
            }
            if ((nodetype==NODETYPE_FOLDER)&&(!virtualFolder)) {
                QDir pdir = QFileInfo(projectFileName_).dir();
                QDir tdir = QFileInfo(projectFileName).dir();
                data["fspath"]=tdir.relativeFilePath(pdir.absoluteFilePath(fspath));
                item->setData(0, Qt::UserRole, data);
            }
        }
    }

    /*
    QTreeWidgetItem* rootitem = topLevelItem(0);
    if (rootitem)
        rootitem->setText(0,QFileInfo(projectFileName).completeBaseName());
*/
    projectFileName_ = projectFileName;
    xmlString_ = toXml();
    bool changed = isModifed_ == true;
    isModifed_ = false;
    if (changed)
        emit modificationChanged(isModifed_);

}

void LibraryTreeWidget::consolidateProject()
{
    //Turn all linked dirs/files into real copies in the project asset folder
    std::stack<QTreeWidgetItem*> stack;
    stack.push(filesRootFolder());

    while (!stack.empty())
    {
        QTreeWidgetItem* item = stack.top();
        stack.pop();

        QMap<QString, QVariant> data=item->data(0, Qt::UserRole).toMap();
        int nodetype = data ["nodetype"].toInt();
        QString fspath = data["fspath"].toString();

        if ((nodetype==NODETYPE_FILES)||(nodetype==NODETYPE_FOLDER)) {
            bool linkFolder=(nodetype!=NODETYPE_FILES);
            getItemPath(item,&linkFolder);
            fspath=getItemPath(item,nullptr,true);

            // Ensure path exists
            QDir dir = QFileInfo(projectFileName_).dir();
            if (!(fspath.isEmpty())) {
                dir.mkdir(fspath);
            }
            if ((!fspath.isEmpty())&&dir.cd(fspath)) {
               QDir pdir = QFileInfo(projectFileName_).dir();
               for (int i = item->childCount()-1; i >=0 ; i--)
                {
                    QTreeWidgetItem* sub = item->child(i);
                    QMap<QString, QVariant> sdata=sub->data(0, Qt::UserRole).toMap();
                    int stype = sdata["nodetype"].toInt();
                    if (stype==NODETYPE_FOLDER) {
                        if (linkFolder) {
                            /*Current folder is either a link or a folder within a linked folder
                             *Fully mark it as a link for recursive pass. */
                            sdata["fspath"]=getItemPath(sub); //Folder is no longer a path
                            sub->setData(0, Qt::UserRole, sdata);
                        }
                        stack.push(sub);
                    }
                    else if ((stype==NODETYPE_FILE)&&(sdata["link"].toBool()||linkFolder))
                    {
                        //Copy linked file into assets, and remove link flag
                        QString oldPath=sdata["filename"].toString();
                        QString newPath=getItemPath(sub,nullptr,true);
                        if (!QFile::copy(pdir.absoluteFilePath(oldPath), pdir.absoluteFilePath(newPath)))
                        {
                            QMessageBox::critical(this, tr("Gideros"), tr("File %1 can't be copied to %2. Cancelling, but some files may have been internalized already.")
                                                  .arg(pdir.absoluteFilePath(oldPath))
                                                  .arg(pdir.absoluteFilePath(newPath)));
                            checkModification();
                            return;
                        }
                        else {
                            sdata["filename"] = newPath;
                            sdata["link"] = false;
                            sub->setData(0, Qt::UserRole, sdata);
                            if (dependencyGraph_.hasCode(oldPath))
                                dependencyGraph_.renameCode(oldPath,newPath);
                        }
                    }
                }
            }
            if (nodetype==NODETYPE_FOLDER) {
                data["fspath"]=""; //Folder is no longer a path
                item->setData(0, Qt::UserRole, data);
            }
        }
    }
    refresh();
    checkModification();
}

void LibraryTreeWidget::dropEvent(QDropEvent *event)
{
    QTreeWidgetItem *item=currentItem();
    QString srcPath=getItemPath(item);
    QTreeWidget::dropEvent(event);
    if (event->isAccepted())
    {
        QString dstPath=getItemPath(item);
        QMap<QString, QVariant> data=item->data(0, Qt::UserRole).toMap();
        int nodetype = data ["nodetype"].toInt();
        if ((!srcPath.isEmpty())&&(!dstPath.isEmpty()))
        {
            QDir dir = QFileInfo(projectFileName_).dir();
            if (nodetype==NODETYPE_FOLDER)
            {
                //Move folder
                dir.rename(srcPath,dstPath);
                refreshFolder(item);
            }
            else if (nodetype==NODETYPE_FILE)
            {
                if (!data["link"].toBool())
                {
                    //Rename file
                    if (dir.rename(srcPath,dstPath))
                    {
                        QFileInfo fileInfo(data["filename"].toString());
                        if (fileInfo.suffix().toLower() == "lua")
                            dependencyGraph_.removeCode(data["filename"].toString());
                        data["filename"]=dstPath;
                        item->setData(0, Qt::UserRole, data);
                        if (fileInfo.suffix().toLower() == "lua")
                            dependencyGraph_.addCode(data["filename"].toString(),data["excludeFromExecution"].toBool());
                    }
                }
            }
        }
    }
}

void LibraryTreeWidget::setModified(bool m)
{
	if (m == false)
        xmlString_ = toXml();

	if (m == isModifed_)
		return;

	isModifed_ = m;

	emit modificationChanged(isModifed_);
}

bool LibraryTreeWidget::isModified() const
{
	return isModifed_;
}

std::vector<std::pair<QString, QString> > LibraryTreeWidget::fileList(bool downsizing,bool webClient, bool justLua) {
    std::vector<std::pair<QString, QString> > result;
    QMap<QString,bool> locked;

    //Add lua plugins
    if (!justLua) {
        QMap<QString, QString> plugins=usedPlugins();
        for (QMap<QString,QString>::const_iterator it=plugins.begin();it!=plugins.end(); it++)
        {
            QFileInfo path(it.value());
            QDir pf=path.dir();
            if (pf.cd("luaplugin"))
            {
                QDir luaplugin_dir = pf.path();
                int root_length = luaplugin_dir.path().length();

                // get all files in luaplugin and any subdirectory of luaplugin
                QDirIterator dir_iter(pf.path(), QStringList() << "*", QDir::Files, QDirIterator::Subdirectories);
                while (dir_iter.hasNext()) {
                    QDir file = dir_iter.next();
                    QString rel_path = file.path().mid(root_length + 1, file.path().length());
                    result.push_back(std::make_pair("_LuaPlugins_/" + rel_path, file.path()));
                    locked["_LuaPlugins_/" + rel_path]=true;
                }
            }
        }
    }
    if (webClient)
    {
        QFileInfo f=QFileInfo("Tools/FBInstant.lua");
        result.push_back(std::make_pair("_LuaPlugins_/FBInstant.lua", f.absoluteFilePath()));
        locked["_LuaPlugins_/FBInstant.lua"]=true;
    }

    QTreeWidgetItem* rootitem = invisibleRootItem();
    rootitem = rootitem->child(0);
    QTreeWidgetItem *pluginsFolder=rootitem?rootitem->child(0):NULL;
    QTreeWidgetItem *filesFolder=rootitem?rootitem->child(1):NULL;
    if (pluginsFolder&&filesFolder) {
        std::vector<QString> dir;

        std::stack<std::pair<QTreeWidgetItem*, bool> > stack;
        stack.push(std::make_pair(filesFolder, true));

        QDir pdir = QFileInfo(projectFileName_).dir();

        while (stack.empty() == false)
        {
            QTreeWidgetItem* item = stack.top().first;
            bool enter = stack.top().second;
            stack.pop();

            if (enter)
            {
                if (item!=filesFolder)
                {
                    QString name = item->text(0);
                    dir.push_back(name);
                    stack.push(std::make_pair(item, false));
                }
                for (int i = 0; i < item->childCount(); ++i)
                {
                    QTreeWidgetItem* childItem = item->child(i);

                    QMap<QString, QVariant> data = childItem->data(0, Qt::UserRole).toMap();
                    QString fileName1 = data["filename"].toString();

                    if (fileName1.isEmpty() == false)
                    {
                        QString fileName = pdir.relativeFilePath(fileName1);
                        bool isLink=data["link"].toBool();
                        bool lock=!isLink;
                        QString name = QFileInfo(fileName).fileName();

                        QString n;
                        for (std::size_t i = 0; i < dir.size(); ++i)
                            n += dir[i] + "/";
                        n += name;

                        if (!(locked[n]||(justLua&&(QFileInfo(fileName).suffix().toLower() != "lua"))))
                        {
                            if (downsizing)
                            {
                                if (data["downsizing"].toBool()) {
                                    result.push_back(std::make_pair(n, fileName));
                                    locked[n]=lock;
                                }
                            }
                            else
                            {
                                result.push_back(std::make_pair(n, fileName));
                                locked[n]=lock;
                            }
                        }
                     }
                    else
                    {
                        //Sub folder
                        stack.push(std::make_pair(childItem, true));
                    }
                }
            }
            else
                dir.pop_back();
        }
    }

    return result;
}

void LibraryTreeWidget::codeDependencies()
{
	QString selected = selectedItems()[0]->data(0, Qt::UserRole).toMap()["filename"].toString();

    std::map<QString,QString> fileMap;
    std::vector<std::pair<QString,QString>> fileQueue=fileList(false,false,false);
    for (std::size_t j = 0; j < fileQueue.size(); ++j)
        fileMap[fileQueue[j].second]=fileQueue[j].first;
    CodeDependenciesDialog codeDependencies(QFileInfo(projectFileName_).dir(),&dependencyGraph_,fileMap, selected, this);

	codeDependencies.exec();
}

bool LibraryTreeWidget::isFileAlreadyImported(const QString& fileName, bool link)
{
	std::stack<QTreeWidgetItem*> stack;
	stack.push(invisibleRootItem());

	while (stack.empty() == false)
	{
		QTreeWidgetItem* item = stack.top();
		stack.pop();

        QMap<QString, QVariant> data=item->data(0, Qt::UserRole).toMap();
        if ((fileName == data["filename"].toString())&&(link==data["link"].toBool()))
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
		int f0 = i0->data(0, Qt::UserRole).toMap()["filename"].toString().isEmpty() ? 0 : 1;		// folders has lower priority (sorts to top)
		int f1 = i1->data(0, Qt::UserRole).toMap()["filename"].toString().isEmpty() ? 0 : 1;

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
    sortFolder(root);
}

void LibraryTreeWidget::sortFolder(QTreeWidgetItem* root)
{
    QList<QTreeWidgetItem*> children = root->takeChildren();

    std::sort(children.begin(), children.end(), LessQTreeWidgetItem());

    root->addChildren(children);
}

QString LibraryTreeWidget::getItemPath(QTreeWidgetItem *item,bool *linkFolder,bool internal)
{
    QMap<QString, QVariant> data=item->data(0, Qt::UserRole).toMap();
    QString fspath = data["fspath"].toString();
    int type = data ["nodetype"].toInt();
    if ((!fspath.isEmpty())&&((!internal)||(type==NODETYPE_FILES)))
    {
        if (linkFolder) *linkFolder=(type!=NODETYPE_FILES);
        return fspath;
    }
    if (linkFolder) *linkFolder=false;
    fspath=item->text(0);
    QTreeWidgetItem* p=item->parent();
    while (p)
    {
        QMap<QString, QVariant> data=p->data(0, Qt::UserRole).toMap();
        int type = data ["nodetype"].toInt();
        if ((type!=NODETYPE_FOLDER)&&(type!=NODETYPE_FILES)) //Trouble: orphaned folder
        {
            fspath=QString();
            break;
        }
        QString path = data["fspath"].toString();
        if ((!path.isEmpty())&&((!internal)||(type==NODETYPE_FILES))) //If we want the internalized path, still accepts root files folder
        {
            if (linkFolder) *linkFolder=(type!=NODETYPE_FILES);
            fspath=path+"/"+fspath;
            break;
        }
        fspath=p->text(0)+"/"+fspath;
        p=p->parent();
    }
    return fspath;
}

void LibraryTreeWidget::addNewFile()
{
    QTreeWidgetItem* root = invisibleRootItem();

    if (selectedItems().empty() == false)
        root = selectedItems().front();

    QDir dir = QFileInfo(projectFileName_).dir();
    if (!dir.cd(getItemPath(root)))
    {
        QMessageBox::critical(this, tr("Gideros"), tr("Directory %1 does not exist.").arg(getItemPath(root)));
        return;
    }
	AddNewFileDialog addNewFile(dir.path(), this);

	while (1)
	{
		if (addNewFile.exec() == QDialog::Accepted)
		{
            /*QDir newdir = dir;
		
			if (newdir.cd(addNewFile.location()) == false)
			{
				QMessageBox::critical(this, tr("Gideros"), tr("Directory %1 does not exist.").arg(addNewFile.location()));
				continue;
            }*/

            QString filename = dir.absoluteFilePath(addNewFile.fileName());

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

            QDir pdir = QFileInfo(projectFileName_).dir();

            QTreeWidgetItem *item = createFileItem(pdir.relativeFilePath(filename),false);
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
	if (!item->childCount()) return QString();
	item = item->child(0);
	if (item->childCount()<2) return QString();
	item = item->child(1);

	for (int i = 0; i < items.size(); ++i)
	{
		item = childWithText(item, 0, items[i]);
		if (item == 0)
			return QString();
	}

	return item->data(0, Qt::UserRole).toMap()["filename"].toString();
}

QString LibraryTreeWidget::itemName(QDir dir,const QString& fileName) const
{
    std::stack<QTreeWidgetItem*> stack;
    stack.push(invisibleRootItem());

    while (stack.empty() == false)
    {
        QTreeWidgetItem* item = stack.top();
        stack.pop();

        QString iname=dir.absoluteFilePath(item->data(0, Qt::UserRole).toMap()["filename"].toString());
        if (QFileInfo(fileName) == QFileInfo(iname))
        {
            QString result;
            while (item->parent()&&item->parent()->parent())
            {
                if (result.isEmpty())
                    result = item->text(0);
                else
                    result = item->text(0) + "/" + result;
                item = item->parent();
            }
            return result;
        }

        for (int i = 0; i < item->childCount(); ++i)
            stack.push(item->child(i));
    }
    return QString();
}

void LibraryTreeWidget::insertIntoDocument()
{
	if (selectedItems().empty() == true)
		return;

	QString result;
	QTreeWidgetItem* item = selectedItems()[0];
	while (item->parent()&&item->parent()->parent())
	{
		if (result.isEmpty())
			result = item->text(0);
		else
			result = item->text(0) + "/" + result;
		item = item->parent();
	}

	emit insertIntoDocument("\"" + result + "\"");
}

void LibraryTreeWidget::projectProperties()
{
	ProjectPropertiesDialog dialog(projectFileName_,&properties_, this);
	dialog.exec();
}

QMap<QString, QString> LibraryTreeWidget::usedPlugins()
{
	QMap<QString, QString> usedPlugins;
	QMap<QString, QString> allPlugins=ProjectProperties::availablePlugins();
	for (QSet<ProjectProperties::Plugin>::const_iterator it=properties_.plugins.begin();it!=properties_.plugins.end(); it++)
	{
		ProjectProperties::Plugin p=*it;
		if (p.enabled)
		{
			QString path=allPlugins[p.name];
			if (!path.isEmpty())
				usedPlugins[p.name]=path;
		}
	}
	return usedPlugins;
}

void LibraryTreeWidget::automaticDownsizing(bool checked)
{
	if (selectedItems().empty() == true)
		return;

	QTreeWidgetItem* item = selectedItems()[0];

	QMap<QString, QVariant> data = item->data(0, Qt::UserRole).toMap();
	data["downsizing"] = checked;
	item->setData(0, Qt::UserRole, data);

	if (checked)
	{
		QDir dir = QFileInfo(projectFileName_).dir();
		QString fileName = item->data(0, Qt::UserRole).toMap()["filename"].toString();
		emit automaticDownsizingEnabled(QDir::cleanPath(dir.absoluteFilePath(fileName)));
	}
}

void LibraryTreeWidget::excludeFromExecution(bool checked)
{
    if (selectedItems().empty() == true)
        return;

    QTreeWidgetItem* item = selectedItems()[0];

    QMap<QString, QVariant> data = item->data(0, Qt::UserRole).toMap();
    data["excludeFromExecution"] = checked;
    item->setData(0, Qt::UserRole, data);

    item->setIcon(0, IconLibrary::instance().icon(0, checked ? "lua with stop" : "lua"));

    dependencyGraph_.setExcludeFromExecution(data["filename"].toString(), checked);
}

void LibraryTreeWidget::excludeFromEncryption(bool checked)
{
    if (selectedItems().empty() == true)
        return;

    QTreeWidgetItem* item = selectedItems()[0];

    QMap<QString, QVariant> data = item->data(0, Qt::UserRole).toMap();
    data["excludeFromEncryption"] = checked;
    item->setData(0, Qt::UserRole, data);

    //item->setIcon(0, IconLibrary::instance().icon(0, checked ? "lua with stop" : "lua"));
    //XXX would be good to overlay a little icon symbol saying that the file won't be encrypted
}

void LibraryTreeWidget::excludeFromPackage(bool checked)
{
    if (selectedItems().empty() == true)
        return;

    QTreeWidgetItem* item = selectedItems()[0];

    QMap<QString, QVariant> data = item->data(0, Qt::UserRole).toMap();
    data["excludeFromPackage"] = checked;
    item->setData(0, Qt::UserRole, data);

    //item->setIcon(0, IconLibrary::instance().icon(0, checked ? "lua with stop" : "lua"));
    //XXX would be good to overlay a little icon symbol saying that the file won't be packaged
}
