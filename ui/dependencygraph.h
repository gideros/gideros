#ifndef DEPENDENCYGRAPH_H
#define DEPENDENCYGRAPH_H

#include <QString>
#include <set>
#include <vector>
#include <map>

class DependencyGraph
{
    struct Vertex
    {
        Vertex(const QString& code, bool excludeFromExecution) :
            code(code),
            excludeFromExecution(excludeFromExecution),
            visited(false)
        {
        }

        QString code;
        bool excludeFromExecution;
        mutable bool visited;
        std::set<Vertex*> dependencies;
    };

public:
    DependencyGraph() {}
    ~DependencyGraph();

    void clear();

    void addCode(const QString& code, bool excludeFromExecution);
    void removeCode(const QString& code);
    void addDependency(const QString& code0, const QString& code1);
    void removeDependency(const QString& code0, const QString& code1);
    bool isDependencyValid(const QString& code0, const QString& code1) const;
    bool isDependent(const QString& code0, const QString& code1) const;
    void setExcludeFromExecution(const QString& code, bool excludeFromExecution);
    std::vector<std::pair<QString, bool> > topologicalSort() const;
    std::vector<QString> codes() const;
    std::vector<std::pair<QString, QString> > dependencies() const;

private:
    void topologicalSortHelper(Vertex* vertex, std::vector<std::pair<QString, bool> >& result) const;

    typedef std::map<std::pair<int, QString>, Vertex*> map;
    typedef map::iterator iterator;
    typedef map::const_iterator const_iterator;
    map vertices_;

private:
    DependencyGraph(const DependencyGraph&);
    DependencyGraph& operator=(const DependencyGraph&);
};

#endif
