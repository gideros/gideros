#ifndef LIBRARYTREEWIDGET_H
#define LIBRARYTREEWIDGET_H

#include <QTreeWidget>
#include <QDragEnterEvent>
#include <QDomDocument>
#include <QDir>

#include "codedependenciesdialog.h"
#include "projectproperties.h"

class LibraryTreeWidget : public QTreeWidget
{
	Q_OBJECT

public:
	static LibraryTreeWidget *lua_instance;
	LibraryTreeWidget(QWidget *parent);
	~LibraryTreeWidget();
    QString projectFileName_;

	QDomDocument toXml() const; 
	void loadXml(const QString& projectFileName, const QDomDocument& doc);
	void clear();
	void newProject(const QString& projectFileName);

	void setModified(bool m);
	bool isModified() const;
	QMap<QString, QString> usedPlugins();

    std::vector<std::pair<QString, bool> > topologicalSort() const
	{
		return dependencyGraph_.topologicalSort();
	}

	QString fileName(const QString& itemName) const;
    QString itemName(QDir dir,const QString& fileName) const;

	const ProjectProperties& getProjectProperties() const
	{
		return properties_;
	}

	ProjectProperties& getProjectProperties()
	{
		return properties_;
	}

    QTreeWidgetItem *newFile(QTreeWidgetItem *parent,QString name,QMap<QString, QVariant> data);
    void newFolder(QTreeWidgetItem *parent,QString name);
    void remove(QTreeWidgetItem *item);
    void refreshFolder(QTreeWidgetItem *item);
    void sortFolder(QTreeWidgetItem* root);
    QString getItemPath(QTreeWidgetItem *item);

signals:
	void modificationChanged(bool m);
	void openRequest(const QString& itemName, const QString& fileName);
	void previewRequest(const QString& itemName, const QString& fileName);
	void insertIntoDocument(const QString& text);
	void automaticDownsizingEnabled(const QString& fileName);

private slots:
	void onCustomContextMenuRequested(const QPoint& pos);
    void onItemDoubleClicked(QTreeWidgetItem* item, int column);
    void onItemChanged(QTreeWidgetItem* item, int column);
    void onCurrentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);

private slots:
	void addNewFile();
	void importToLibrary();
	void importFolder();
//	void newFont();
	void newFolder();
	void remove();
    void rename();
    void refresh();
    void sort();
	void insertIntoDocument();
	void projectProperties();
    void automaticDownsizing(bool checked);
    void excludeFromExecution(bool checked);
    void excludeFromEncryption(bool checked);
    void excludeFromPackage(bool checked);
    void showInFinder();
    void addPlugin();
    void propPlugin();
private slots:
	void checkModification();

private:
    QTreeWidgetItem* createFileItem(const QString& file, bool link, bool downsizing = false, bool excludeFromExecution = false, bool excludeFromEncryption = false, bool excludeFromPackage = false);
	QTreeWidgetItem* createFolderItem(const QString& name, const QString& fspath);
	QTreeWidgetItem* createProjectItem(const QString& name);
	QTreeWidgetItem* createCatFolderItem(const QString& name, const QString& fspath, const QString& icon, int nodetype, bool drop=false);
    QTreeWidgetItem* createPluginItem(const QString& name);
    bool hasItemNamed(QTreeWidgetItem* root,QString name);

private:
	QAction* addNewFileAction_;
	QAction* importToLibraryAction_;
	QAction* importFolderAction_;
//	QAction* newFontAction_;
	QAction* newFolderAction_;
	QAction* removeAction_;
    QAction* renameAction_;		// only for folders
    QAction* refreshAction_;		// only for folders
    QAction* codeDependenciesAction_;
	QAction* sortAction_;
	QAction* insertIntoDocumentAction_;
	QAction* projectPropertiesAction_;
	QAction* automaticDownsizingAction_;
    QAction* excludeFromExecutionAction_;
    QAction* excludeFromEncryptionAction_;
    QAction* excludeFromPackageAction_;
    QAction* showInFindeAction_;
    QAction* addPluginAction_;
    QAction* propPluginAction_;
private:
	QString xmlString_;
	bool isModifed_;
protected:
    virtual void dropEvent(QDropEvent *event) override;
private:
	DependencyGraph dependencyGraph_;

private slots:
	void codeDependencies();

private:
	bool isFileAlreadyImported(const QString& fileName);

private:
	ProjectProperties properties_;
};

#endif // LIBRARYTREEWIDGET_H
