#include "codedependenciesdialog.h"
#include <algorithm>
#include <stack>

DependencyGraph::~DependencyGraph()
{
	clear();
}

void DependencyGraph::clear()
{
	for (const_iterator iter = vertices_.begin(); iter != vertices_.end(); ++iter)
		delete iter->second;

	vertices_.clear();
}

void DependencyGraph::addCode(const QString& code)
{
	vertices_[code] = new Vertex(code);
}

void DependencyGraph::removeCode(const QString& code)
{
	Vertex* vertex = vertices_.find(code)->second;

	for (iterator iter = vertices_.begin(); iter != vertices_.end(); ++iter)
		iter->second->dependencies.erase(vertex);

	vertices_.erase(code);

	delete vertex;
}

void DependencyGraph::addDependency(const QString& code0, const QString& code1)
{
	Vertex* vertex0 = vertices_.find(code0)->second;
	Vertex* vertex1 = vertices_.find(code1)->second;

	vertex0->dependencies.insert(vertex1);
}

void DependencyGraph::removeDependency(const QString& code0, const QString& code1)
{
	Vertex* vertex0 = vertices_.find(code0)->second;
	Vertex* vertex1 = vertices_.find(code1)->second;

	vertex0->dependencies.erase(vertex1);
}

bool DependencyGraph::isDependent(const QString& code0, const QString& code1) const
{
	Vertex* vertex0 = vertices_.find(code0)->second;
	Vertex* vertex1 = vertices_.find(code1)->second;

	return vertex0->dependencies.find(vertex1) != vertex0->dependencies.end();
}


bool DependencyGraph::isDependencyValid(const QString& code0, const QString& code1) const
{
//	if (isDependent(code0, code1) == true)
//		return true;
	Vertex* vertex0 = vertices_.find(code0)->second;
	Vertex* vertex1 = vertices_.find(code1)->second;

	for (const_iterator iter = vertices_.begin(); iter != vertices_.end(); ++iter)
		iter->second->visited = false;

	std::stack<Vertex*> stack;

	stack.push(vertex1);
	stack.push(vertex0);

	while (stack.empty() == false)
	{
		Vertex* vertex = stack.top();
		stack.pop();

		if (vertex->visited == true)
		{
			if (vertex == vertex0)
				return false;

			continue;
		}

		vertex->visited = true;

		for (std::set<Vertex*>::iterator iter = vertex->dependencies.begin(); iter != vertex->dependencies.end(); ++iter)
			stack.push(*iter);
	}

	return true;
}

std::vector<QString> DependencyGraph::topologicalSort() const
{
	std::vector<QString> result;

	for (const_iterator iter = vertices_.begin(); iter != vertices_.end(); ++iter)
		iter->second->visited = false;

	for (const_iterator iter = vertices_.begin(); iter != vertices_.end(); ++iter)
		topologicalSortHelper(iter->second, result);

	return result;
}

void DependencyGraph::topologicalSortHelper(Vertex* vertex, std::vector<QString>& result) const
{
	if (vertex->visited == true)
		return;

	vertex->visited = true;

	for (std::set<Vertex*>::iterator iter = vertex->dependencies.begin(); iter != vertex->dependencies.end(); ++iter)
		topologicalSortHelper(*iter, result);

	result.push_back(vertex->code);
}

std::vector<QString> DependencyGraph::codes() const
{
	std::vector<QString> result;

	for (const_iterator iter = vertices_.begin(); iter != vertices_.end(); ++iter)
		result.push_back(iter->second->code);

	return result;
}

std::vector<std::pair<QString, QString> > DependencyGraph::dependencies() const
{
	std::vector<std::pair<QString, QString> > result;

	for (const_iterator iter = vertices_.begin(); iter != vertices_.end(); ++iter)
	{
		Vertex* vertex = iter->second;

		for (std::set<Vertex*>::iterator iter = vertex->dependencies.begin(); iter != vertex->dependencies.end(); ++iter)
			result.push_back(std::make_pair(vertex->code, (*iter)->code));
	}

	return result;
}


CodeDependenciesDialog::CodeDependenciesDialog(DependencyGraph* graph, const QString& selected, QWidget* parent) :
	QDialog(parent)
{
	ui.setupUi(this);
	this->graph = graph;

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
	std::vector<QString> codes = graph->topologicalSort();
	ui.callOrder->clear();
	for (std::size_t i = 0; i < codes.size(); ++i)
		ui.callOrder->addItem(codes[i]);
}
