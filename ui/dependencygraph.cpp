#include "dependencygraph.h"
#include <stack>

inline std::pair<int, QString> _(const QString& str)
{
    static QString init("init.lua");
    static QString main("main.lua");

    if (str == init)
        return std::make_pair(0, str);
    if (str == main)
        return std::make_pair(2, str);
    return std::make_pair(1, str);
}

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

void DependencyGraph::addCode(const QString& code, bool excludeFromExecution)
{
    vertices_[_(code)] = new Vertex(code, excludeFromExecution);
}

void DependencyGraph::removeCode(const QString& code)
{
    Vertex* vertex = vertices_.find(_(code))->second;

    for (iterator iter = vertices_.begin(); iter != vertices_.end(); ++iter)
        iter->second->dependencies.erase(vertex);

    vertices_.erase(_(code));

    delete vertex;
}

void DependencyGraph::addDependency(const QString& code0, const QString& code1)
{
    Vertex* vertex0 = vertices_.find(_(code0))->second;
    Vertex* vertex1 = vertices_.find(_(code1))->second;

    vertex0->dependencies.insert(vertex1);
}

void DependencyGraph::removeDependency(const QString& code0, const QString& code1)
{
    Vertex* vertex0 = vertices_.find(_(code0))->second;
    Vertex* vertex1 = vertices_.find(_(code1))->second;

    vertex0->dependencies.erase(vertex1);
}

bool DependencyGraph::isDependent(const QString& code0, const QString& code1) const
{
    Vertex* vertex0 = vertices_.find(_(code0))->second;
    Vertex* vertex1 = vertices_.find(_(code1))->second;

    return vertex0->dependencies.find(vertex1) != vertex0->dependencies.end();
}


bool DependencyGraph::isDependencyValid(const QString& code0, const QString& code1) const
{
//	if (isDependent(code0, code1) == true)
//		return true;
    Vertex* vertex0 = vertices_.find(_(code0))->second;
    Vertex* vertex1 = vertices_.find(_(code1))->second;

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

void DependencyGraph::setExcludeFromExecution(const QString& code, bool excludeFromExecution)
{
    vertices_[_(code)]->excludeFromExecution = excludeFromExecution;
}

std::vector<std::pair<QString, bool> > DependencyGraph::topologicalSort() const
{
    std::vector<std::pair<QString, bool> > result;

    for (const_iterator iter = vertices_.begin(); iter != vertices_.end(); ++iter)
        iter->second->visited = false;

    for (const_iterator iter = vertices_.begin(); iter != vertices_.end(); ++iter)
        topologicalSortHelper(iter->second, result);

    return result;
}

void DependencyGraph::topologicalSortHelper(Vertex* vertex, std::vector<std::pair<QString, bool> >& result) const
{
    if (vertex->visited == true)
        return;

    vertex->visited = true;

    for (std::set<Vertex*>::iterator iter = vertex->dependencies.begin(); iter != vertex->dependencies.end(); ++iter)
        topologicalSortHelper(*iter, result);

    result.push_back(std::make_pair(vertex->code, vertex->excludeFromExecution));
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
