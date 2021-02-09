#include "dependencygraph.h"
#include <stack>
#include <QFile>
#include <QDir>
#include <QTextStream>

void DependencyGraph::Vertex::parseTags(QDir projectDir,const DependencyGraph *graph,std::map<QString, QString> fileMap) {
	excludeFromExecutionTag=false;
	dependenciesTag.clear();
    QFile inputFile(projectDir.filePath(code));
    if (inputFile.open(QIODevice::ReadOnly))
    {
       QTextStream in(&inputFile);
       while (!in.atEnd())
       {
          QString line = in.readLine().trimmed();
          if (line.startsWith("--!NEEDS:")) {
                line=line.mid(9);
                if (!line.startsWith("/")) {//Relative path
                    QString thisFile=fileMap[code];
                    int lc=thisFile.lastIndexOf('/');
                    if (lc>=0)
                        line=thisFile.mid(0,lc+1)+line;
                }
                else
                    line=line.mid(1);
                line=QDir::cleanPath(line);
                Vertex *match=nullptr;
                for (std::map<QString,QString>::iterator it=fileMap.begin();it!=fileMap.end();it++)
                    if (it->second==line) { match=graph->getVertex(it->first); break; }
                if (match)
                    dependenciesTag.insert(match);
          }
          else if (line=="--!NOEXEC")
              excludeFromExecutionTag=true;
       }
       inputFile.close();
    }
}

inline std::pair<int, QString> _(const QString& str)
{
    static QString init("init.lua");
    static QString main("main.lua");
    static QString init2("assets/init.lua");
    static QString main2("assets/main.lua");

    if ((str == init)||(str==init2))
        return std::make_pair(0, str);
    if ((str == main)||(str==main2))
        return std::make_pair(2, str);
    return std::make_pair(1, str);
}

DependencyGraph::~DependencyGraph()
{
    clear();
}

DependencyGraph::Vertex *DependencyGraph::getVertex(const QString& code) const
{
    const_iterator it;
    if ((it=vertices_.find(_(code)))==vertices_.end()) return NULL;
    return it->second;
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

bool DependencyGraph::hasCode(const QString& code)
{
    return vertices_.find(_(code))!=vertices_.end();
}

void DependencyGraph::renameCode(const QString& oldCode,const QString& newCode)
{
    if (oldCode==newCode) return;
    vertices_[_(newCode)]=vertices_[_(oldCode)];
    vertices_[_(newCode)]->code=newCode;
    vertices_.erase(_(oldCode));
}

void DependencyGraph::removeCode(const QString& code)
{
    Vertex* vertex = vertices_.find(_(code))->second;

    for (iterator iter = vertices_.begin(); iter != vertices_.end(); ++iter)
        iter->second->dependencies.erase(vertex);
    for (iterator iter = vertices_.begin(); iter != vertices_.end(); ++iter)
        iter->second->dependenciesTag.erase(vertex);

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

std::vector<std::pair<QString, bool> > DependencyGraph::topologicalSort(QDir projectDir,std::map<QString, QString> fileMap) const
{
    std::vector<std::pair<QString, bool> > result;

    for (const_iterator iter = vertices_.begin(); iter != vertices_.end(); ++iter) {
        iter->second->parseTags(projectDir,this,fileMap);
        iter->second->visited = false;
    }

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

    for (std::set<Vertex*>::iterator iter = vertex->dependenciesTag.begin(); iter != vertex->dependenciesTag.end(); ++iter)
        topologicalSortHelper(*iter, result);

    result.push_back(std::make_pair(vertex->code, vertex->excludeFromExecution||vertex->excludeFromExecutionTag));
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
