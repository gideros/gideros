#include "codedependenciesdialog.h"
#include <algorithm>
#include <stack>

CodeDependenciesDialog::CodeDependenciesDialog(QDir projectDir,DependencyGraph* graph, std::map<QString,QString> fileMap, const QString& selected, QWidget* parent) :
	QDialog(parent)
{
	ui.setupUi(this);
	this->graph = graph;
    this->fileMap=fileMap;
    this->projectDir=projectDir;

	std::vector<QString> codes = graph->codes();

	int index = 0;
	for (std::size_t i = 0; i < codes.size(); ++i)
	{
		ui.codes->addItem(codes[i]);
		
		if (selected == codes[i])
			index = i;
	}

	ui.codes->setCurrentIndex(index);

	fillDepends(selected);
	updateCallOrder();

	connect(ui.dependsOn, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(itemChanged(QListWidgetItem*)));
	connect(ui.codes, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(currentIndexChanged(const QString&)));
}

CodeDependenciesDialog::~CodeDependenciesDialog()
{

}

void CodeDependenciesDialog::fillDepends(const QString& code0)
{
	ui.dependsOn->clear();

	std::vector<QString> codes = graph->codes();

	for (std::size_t i = 0; i < codes.size(); ++i)
	{
		QString code1 = codes[i];

		QListWidgetItem* item = new QListWidgetItem(codes[i]);

		if (graph->isDependent(code0, code1) == true)
			item->setCheckState(Qt::Checked);
		else
		{
			item->setCheckState(Qt::Unchecked);
			if (graph->isDependencyValid(code0, code1) == false)
				item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
		}

		ui.dependsOn->addItem(item);
	}
}

void CodeDependenciesDialog::itemChanged(QListWidgetItem* item)
{
	QString code0 = ui.codes->currentText();
	QString code1 = item->text();

	switch (item->checkState())
	{
	case Qt::Unchecked:
		graph->removeDependency(code0, code1);
		break;
	case Qt::PartiallyChecked:
		break;
	case Qt::Checked:
		graph->addDependency(code0, code1);
		break;
	}

#if 0
	disconnect(ui.dependsOn, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(itemChanged(QListWidgetItem*)));
	for (int i = 0; i < ui.dependsOn->count(); ++i)
	{
		QListWidgetItem* item = ui.dependsOn->item(i);
		QString code1 = item->text();

		if (graph->isDependencyValid(code0, code1) == true)
			item->setFlags(item->flags() | Qt::ItemIsEnabled);
		else
			item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
	}
	connect(ui.dependsOn, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(itemChanged(QListWidgetItem*)));
#endif

	updateCallOrder();
}

void CodeDependenciesDialog::currentIndexChanged(const QString& text)
{
	fillDepends(text);
}

void CodeDependenciesDialog::updateCallOrder()
{
    std::vector<std::pair<QString, bool> > codes = graph->topologicalSort(projectDir,fileMap);
	ui.callOrder->clear();
	for (std::size_t i = 0; i < codes.size(); ++i)
    {
        QListWidgetItem* item = new QListWidgetItem(codes[i].first);
        if (codes[i].second)
            item->setForeground(Qt::gray);
        ui.callOrder->addItem(item);
    }
}
