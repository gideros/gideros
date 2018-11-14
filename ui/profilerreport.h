#ifndef PROFILERREPORT_H
#define PROFILERREPORT_H

#include <QVariant>
#include <QString>
#include <QTemporaryDir>
class ProfilerReport
{
    static int proNum;
public:
    static void displayReport(QVariant pinfo);
    static QString generateReport(QVariant pinfo);
};

#endif // PROFILERREPORT_H
