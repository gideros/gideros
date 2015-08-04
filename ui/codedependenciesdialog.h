#ifndef CODEDEPENDENCIESDIALOG_H
#define CODEDEPENDENCIESDIALOG_H

#include <QDialog>
#include "ui_codedependenciesdialog.h"
#include <QString>
#include <map>
#include <vector>
#include <set>
#include "dependencygraph.h"

class CodeDependenciesDialog : public QDialog
{
	Q_OBJECT

public:
	CodeDependenciesDialog(DependencyGraph* graph, const QString& selected, QWidget* parent = 0);
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
};

#endif // CODEDEPENDENCIESDIALOG_H
