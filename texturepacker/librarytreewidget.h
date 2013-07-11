#ifndef LIBRARYTREEWIDGET_H
#define LIBRARYTREEWIDGET_H

#include <QTreeWidget>
class QAction;
#include <QDomDocument>
#include "projectproperties.h"

class LibraryTreeWidget : public QTreeWidget
{
	Q_OBJECT

public:
	LibraryTreeWidget(QWidget* parent = 0);
	~LibraryTreeWidget();

	QDomDocument toXml() const;

	void newProject(const QString& projectFileName);
	void loadXml(const QString& projectFileName, const QDomDocument& doc);

	void clear();

	void setWindowModified(bool m);

	ProjectProperties& getProjectProperties();

signals:
	void modificationChanged(bool m);
	void changed();

private slots:
	void importToLibrary();
	void newFolder();
	void remove();
	void rename();
	void sort();


private slots:
	void onCustomContextMenuRequested(const QPoint& pos);
	void checkModification();

private:
	QAction* importToLibraryAction_;
	QAction* newFolderAction_;
	QAction* removeAction_;
	QAction* renameAction_;		// only for folders
	QAction* sortAction_;		// only for folders and root

private:
	QString projectFileName_;

private:
	bool isFileAlreadyImported(const QString& fileName);
	QTreeWidgetItem* createFileItem(const QString& file);
	QTreeWidgetItem* createFolderItem(const QString& name);
	QTreeWidgetItem* createProjectItem(const QString& name);

private:
	QString xmlString_;

private:
	ProjectProperties properties_;
};

#endif // LIBRARYTREEWIDGET_H
