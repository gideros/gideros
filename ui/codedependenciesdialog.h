#ifndef CODEDEPENDENCIESDIALOG_H
#define CODEDEPENDENCIESDIALOG_H

#include <QDialog>
#include "ui_codedependenciesdialog.h"
#include <QString>
#include <QDir>
#include <map>
#include <vector>
#include <set>
#include "dependencygraph.h"

class CodeDependenciesDialog : public QDialog
{
	Q_OBJECT

public:
    CodeDependenciesDialog(QDir projectDir,DependencyGraph* graph, std::map<QString,QString> fileMap, const QString& selected, QWidget* parent = 0);
	~CodeDependenciesDialog();

private slots:
	void itemChanged(QListWidgetItem* item);
	void currentIndexChanged(const QString& text);

private:
	Ui::CodeDependenciesDialogClass ui;

private:
	void fillDepends(const QString& code);
	void updateCallOrder();

private:
	DependencyGraph* graph;
    std::map<QString,QString> fileMap;
    QDir projectDir;
};

#endif // CODEDEPENDENCIESDIALOG_H
