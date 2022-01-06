#include "profilerreport.h"
#include <mainwindow.h>
#include <QTextStream>
#include <QDesktopServices>
#include <QUrl>

int ProfilerReport::proNum=0;

void ProfilerReport::displayReport(QVariant pinf) {
    if (MainWindow::tempDir->isValid()) {
        QString rpt=generateReport(pinf);
        QFile tmpFile(MainWindow::tempDir->filePath(QString("profiler")+QString::number(++proNum)+".html"));
        tmpFile.open(QIODevice::WriteOnly);
        QTextStream out( &tmpFile );
        out << rpt;
        tmpFile.close();
        QDesktopServices::openUrl( QUrl::fromLocalFile( tmpFile.fileName() ) );
    }
}

QString ProfilerReport::generateReport(QVariant pinf) {
    QMap<QString,QVariant> pinfo=pinf.value<QMap<QString,QVariant>>();
    QStringList roots;
    QStringList funcs;
    QMap<QString,QStringList> callees;

    QString out;
    out+="<html><head><style type=\"text/css\">\n"
            "p {font-family: \"Lucida Console\", Courier New; font-weight:bold;}\n"
            "table, th, td, tr { border-collapse: collapse; padding: 3px; font-family: \"Lucida Console\", Courier New; font-weight:bold;}\n"
            "tr:nth-child(even) {background-color: #f8f8f8;}\n"
            "tr:nth-child(odd) {background-color: #ffffff;}\n"
            "tr:hover {background-color: #e0e0e0;}\n"
            "a {color:Crimson; text-decoration: none;}\n"
            ".caller, .callerZ { color:DodgerBlue; border: 0px; }\n"
            ".callee, .calleeZ { color:Green; }\n"
            ".current, .currentZ { color:FireBrick;; font-weight: bold; }\n"
            ".sep { background-color: #fff; }\n";
    out+="</style><script>";
    out+="function toggle(cls, on) {\n"
        "var lst = document.getElementsByClassName(cls);\n"
        "for(var i = 0; i < lst.length; ++i) {\n"
        "    lst[i].style.display = on ? '' : 'none';\n"
        "}\n"
    "}\n"
    "function toggle3(s)\n"
    "{\n"
    "toggle(\"calleeZ\",s);\n"
    "toggle(\"callerZ\",s);\n"
    "toggle(\"currentZ\",s);\n"
    "}\n";
    out+="</script></head><body>";
    out+="<p><b>GiderosSDK Profiler:</b></p>"
    "<p><span class=\"callee\" onClick=\"toggle3(true);\">Show All</span> | <span class=\"current\" onClick=\"toggle3(false);\">Show above 0%</span></p>";

    out+="<table><thead><tr class=\"theader\">\n";
    out+="<th class=\"fnum\">#</th><th class=\"ftime\">Time</th><th class=\"fpct\">Ratio</th>";
    out+="<th class=\"fcount\">Count</th><th class=\"fname\">Function</th><th class=\"floc\">Location</th>";
    out+="</tr></thead><tbody>\n";
    QMapIterator<QString, QVariant> it(pinfo);
    while (it.hasNext()) {
        it.next();
        bool isRoot=true;
        QMapIterator<QString, QVariant> cvit(it.value().value<QMap<QString,QVariant>>()["callers"].value<QMap<QString,QVariant>>());
        while (cvit.hasNext()) {
            cvit.next();
            callees[cvit.key()] << it.key();
            isRoot=false;
        }
        if (isRoot) roots << it.key();
        funcs << it.key();
    }

    std::sort(funcs.begin(), funcs.end(),
          [&](const QString& a, const QString &b) ->
          bool {
            return pinfo[a].value<QMap<QString,QVariant>>()["time"].toString().toDouble()>pinfo[b].value<QMap<QString,QVariant>>()["time"].toString().toDouble();
           });
    QMap<QString,int> pmap;
    for (int i=0;i<funcs.size();i++)
        pmap[funcs[i]]=i+1;


    for (int i=0;i<funcs.size();i++)
    {
        QMap<QString,QVariant> p=pinfo[funcs[i]].value<QMap<QString,QVariant>>();
        double ptime=p["time"].toString().toDouble();
        int pcount=p["count"].toString().toInt();
        QMapIterator<QString, QVariant> cvit(p["callers"].value<QMap<QString,QVariant>>());
        while (cvit.hasNext()) {
            cvit.next();
            QString f=cvit.key();
            QMap<QString,QVariant> i=cvit.value().value<QMap<QString,QVariant>>();
            double itime=i["time"].toString().toDouble();
            int icount=i["count"].toString().toInt();
            double ipct=itime*100/ptime;
            out += QString::asprintf("<tr class=\"%s\"><td></td><td>%6.0f</td><td>%3.0f%%</td><td>%d</td><td><a href=\"#f%d\">%s</a></td><td>%s</td></tr>\n",
                                     (ipct<1)?"callerZ":"caller",itime*1000,ipct,icount,pmap[f],pinfo[f].value<QMap<QString,QVariant>>()["name"].toString().toUtf8().constData(),f.toUtf8().constData());
        }
        double otime=0;
        QStringList pcallees=callees[funcs[i]];
        foreach (QString f, pcallees) {
            QMap<QString,QVariant> ii=pinfo[f].value<QMap<QString,QVariant>>()["callers"].value<QMap<QString,QVariant>>()[funcs[i]].value<QMap<QString,QVariant>>();
            double itime=ii["time"].toString().toDouble();
            otime+=itime;
        }
        double ipct=(ptime-otime)*100/ptime;
        out += QString::asprintf("<tr class=\"%s\"><td><a id=\"f%d\">[%d]</a></td><td>%6.0f</td><td>%3.0f%%</td><td>%d</td><td>%s</td><td>%s</td></tr>\n",
                                 "current",i+1,i+1,ptime*1000,ipct,pcount,pinfo[funcs[i]].value<QMap<QString,QVariant>>()["name"].toString().toUtf8().constData(),funcs[i].toUtf8().constData());
        foreach (QString f, pcallees) {
            QMap<QString,QVariant> ii=pinfo[f].value<QMap<QString,QVariant>>()["callers"].value<QMap<QString,QVariant>>()[funcs[i]].value<QMap<QString,QVariant>>();
            double itime=ii["time"].toString().toDouble();
            int icount=ii["count"].toString().toInt();
            double ipct=itime*100/ptime;
            out += QString::asprintf("<tr class=\"%s\"><td></td><td>%6.0f</td><td>%3.0f%%</td><td>%d</td><td><a href=\"#f%d\">%s</a></td><td>%s</td></tr>\n",
                                     (ipct<1)?"calleeZ":"callee",itime*1000,ipct,icount,pmap[f],pinfo[f].value<QMap<QString,QVariant>>()["name"].toString().toUtf8().constData(),f.toUtf8().constData());
        }
        out += "<tr class=\"sep\"><td colspan=6>&nbsp;</td></tr>\n";
    }
    out+="</tbody></table></body></html>";
    return out;
}
