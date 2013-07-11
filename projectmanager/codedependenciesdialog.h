#ifndef CODEDEPENDENCIESDIALOG_H
#define CODEDEPENDENCIESDIALOG_H

#include <QDialog>
#include "ui_codedependenciesdialog.h"
#include <QString>
#include <map>
#include <vector>
#include <set>

class DependencyGraph
{
	struct Vertex
	{
		Vertex(const QString& code) : code(code), visited(false) {}

		QString code;
		mutable bool visited;
		std::set<Vertex*> dependencies;
	};

public:
	DependencyGraph() {}
	~DependencyGraph();

	void clear();

	void addCode(const QString& code);
	void removeCode(const QString& code);
	void addDependency(const QString& code0, const QString& code1);
	void removeDependency(const QString& code0, const QString& code1);
	bool isDependencyValid(const QString& code0, const QString& code1) const;
	bool isDependent(const QString& code0, const QString& code1) const;
	std::vector<QString> topologicalSort() const;
	std::vector<QString> codes() const;
	std::vector<std::pair<QString, QString> > dependencies() const;

private:
	void topologicalSortHelper(Vertex* vertex, std::vector<QString>& result) const;

	typedef std::map<QString, Vertex*> map;
	typedef map::iterator iterator;
	typedef map::const_iterator const_iterator;
	map vertices_;

private:
	DependencyGraph(const DependencyGraph&);
	DependencyGraph& operator=(const DependencyGraph&);
};

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
