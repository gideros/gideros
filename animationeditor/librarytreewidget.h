#ifndef LIBRARYTREEWIDGET_H
#define LIBRARYTREEWIDGET_H

#include <QTreeWidget>
#include <QDragEnterEvent>
#include <QDomDocument>

#include "codedependenciesdialog.h"

class LibraryTreeWidget : public QTreeWidget
{
	Q_OBJECT

public:
	LibraryTreeWidget(QWidget *parent);
	~LibraryTreeWidget();

	QDomDocument toXml() const; 
	void loadXml(const QString& projectFileName, const QDomDocument& doc);
	void clear();
	void setProjectFileName(const QString& fileName);

	void setModified(bool m);
	bool isModified() const;

	std::vector<QString> topologicalSort() const
	{
		return dependencyGraph_.topologicalSort();
	}

	QString fileName(const QString& itemName) const;

signals:
	void modificationChanged(bool m);
	void openRequest(const QString& itemName, const QString& fileName);
	void previewRequest(const QString& itemName, const QString& fileName);

private slots:
	void onCustomContextMenuRequested(const QPoint& pos);
	void onItemDoubleClicked(QTreeWidgetItem* item, int column);
	void onCurrentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);

private slots:
	void addNewFile();
	void importToLibrary();
//	void newFont();
	void newFolder();
	void remove();
	void rename();
	void sort();

private slots:
	void checkModification();

private:
	QTreeWidgetItem* createFileItem(const QString& file);
	QTreeWidgetItem* createFolderItem(const QString& name);

private:
	QAction* addNewFileAction_;
	QAction* importToLibraryAction_;
//	QAction* newFontAction_;
	QAction* newFolderAction_;
	QAction* removeAction_;
	QAction* renameAction_;		// only for folders
	QAction* codeDependenciesAction_;
	QAction* sortAction_;

private:
	QString xmlString_;
	bool isModifed_;
	QString projectFileName_;

private:
	DependencyGraph dependencyGraph_;

private slots:
	void codeDependencies();

private:
	bool isFileAlreadyImported(const QString& fileName);
};

#endif // LIBRARYTREEWIDGET_H
