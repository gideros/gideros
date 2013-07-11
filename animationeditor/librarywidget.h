#ifndef LIBRARYWIDGET_H
#define LIBRARYWIDGET_H

#include <QWidget>
#include "ui_librarywidget.h"
#include <QDomDocument>

class QAction;

class LibraryWidget : public QWidget
{
	Q_OBJECT

public:
	LibraryWidget(QWidget *parent = 0);
	~LibraryWidget();

	QDomDocument toXml() const;
	void loadXml(const QString& projectFileName, const QDomDocument& doc);
	void clear();
	void setProjectFileName(const QString& fileName);

	void setModified(bool m);
	bool isModified() const;

	std::vector<QString> topologicalSort() const
	{
		return ui.treeWidget->topologicalSort();
	}

	QString fileName(const QString& itemName) const;


signals:
	void openRequest(const QString& itemName, const QString& fileName);
	void modificationChanged(bool m);
	void previewRequest(const QString& itemName, const QString& fileName);

private:
	Ui::LibraryWidgetClass ui;
};

#endif // LIBRARYWIDGET_H
