/*
 * Utilities.cpp
 *
 *  Created on: 22 janv. 2016
 *      Author: Nico
 */

#include "Utilities.h"
#include <QProcess>
#include <QFile>
#include "ExportCommon.h"

QString Utilities::RemoveSpaces(QString text,bool allowUnderscore)
{
	QString res;
    if (allowUnderscore)
    {
    	res=text;
        for (int i = 0; i < res.size(); ++i)
        {
            char c = res[i].toLatin1();

            bool number = ('0' <= c) && (c <= '9');
            bool upper = ('A' <= c) && (c <= 'Z');
            bool lower = ('a' <= c) && (c <= 'z');

            if ((!number && !upper && !lower) || (number && i == 0))
            	res[i] = QChar('_');
        }
    }
    else
    {
        // 1234 Hebe Gube 456 --> HebeGube456
        bool letter = false;
        for (int i = 0; i < text.size(); ++i)
        {
            char c = text[i].toLatin1();

            bool number = ('0' <= c) && (c <= '9');
            bool upper = ('A' <= c) && (c <= 'Z');
            bool lower = ('a' <= c) && (c <= 'z');

            if (upper || lower)
                letter = true;

            if ((number || upper || lower) && letter)
            	res += text[i];
        }
    }
    return res;
}

bool Utilities::bitwiseMatchReplace(unsigned char *b,int bo,const unsigned char *m,int ms,const unsigned char *r)
{
	 if (!bo) //Simple case, no bit offset
	 {
		 if (memcmp(b,m,ms))
				 return false;
		 memcpy(b,r,ms);
		 return true;
	 }
	 for (int k=0;k<ms;k++)
	 {
		 unsigned char b1=(b[k]|(b[k+1]<<8))>>bo;
		 if (b1!=m[k]) return false;
	 }
	 for (int k=0;k<ms;k++)
	 {
		 b[k]&=(0xFF>>(8-bo));
		 b[k]|=r[k]<<bo;
		 b[k+1]&=(0xFF<<bo);
		 b[k+1]|=(r[k]>>(8-bo));
	 }
	 return true;
}

int Utilities::bitwiseReplace(char *b,int bs,const char *m,int ms,const char *r,int rs)
{
 if (!memcmp(m,r,ms))
	return 0; //No actual replacement needed
 int rcount=0;
 unsigned char *ub=(unsigned char *)b;
 const unsigned char *um=(const unsigned char *)m;
 const unsigned char *ur=(const unsigned char *)r;
 long int bmo=bs-ms;
 bmo=bmo*8;
 if (ms<=2)
 {
 	for (long int k=0;k<bmo;k++)
	 {
		 if (bitwiseMatchReplace(ub+(k>>3),k&7,um,ms,ur))
	 	{
			 k+=(ms*8)-1;
			rcount++;
		 }
 	}
 }
 else
 {
  unsigned char bshift[8];
  for (int k=0;k<8;k++)
	bshift[k]=(um[1]|(um[2]<<8))>>k;
  for (int k=1;k<(bs+1-ms);k++)
  {
   for (int bo=0;bo<8;bo++)
    if (ub[k]==bshift[bo])
   	if (bitwiseMatchReplace(ub+k-1,(8-bo)&7,um,ms,ur))
   	{
    		k+=ms-1;
		bo=8;
    		rcount++;
   	}
  }
 }
 return rcount;
}

 void Utilities::fileCopy(	const QString& srcName,
                        const QString& destName,
                        const QList<QStringList>& wildcards,
                        const QList<QList<QPair<QByteArray, QByteArray> > >& replaceList)
{
    QString srcName2 = QFileInfo(srcName).fileName();

    int match = -1;
    for (int j = 0; j < wildcards.size(); ++j)
    {
        bool submatch = false;
        for (int i = 0; i < wildcards[j].size(); ++i)
        {
            QRegExp rx(wildcards[j][i]);
            rx.setPatternSyntax(QRegExp::Wildcard);
            if (rx.exactMatch(srcName2))
            {
                submatch = true;
                break;
            }
        }
        if (submatch)
        {
            match = j;
            break;
        }
    }

    if (match != -1)
    {
        ExportCommon::exportInfo("Updating %s\n",destName.toStdString().c_str());
        QFile in(srcName);
        if (!in.open(QFile::ReadOnly))
            return;
        QByteArray data = in.readAll();
        in.close();
	int rcount=0;

        for (int i = 0; i < replaceList[match].size(); ++i)
        	if (replaceList[match][i].first.size()==replaceList[match][i].second.size()) //Perform bitwise replacement if sizes are equal
        		rcount+=bitwiseReplace(data.data(),data.size(),
        			replaceList[match][i].first.constData(),replaceList[match][i].first.size(),
        			replaceList[match][i].second.constData(),replaceList[match][i].second.size());
        	else
		{
        		data.replace(replaceList[match][i].first, replaceList[match][i].second);
			rcount++;
		}

        QFile out(destName);
        if (!out.open(QFile::WriteOnly))
            return;
        out.write(data);
        out.close();
        ExportCommon::exportInfo("Updated:%d replacements\n",rcount);
    }
    else if (QFileInfo(srcName)!=QFileInfo(destName))
    {
        ExportCommon::exportInfo("Copying %s\n",destName.toStdString().c_str());
        QFile::remove(destName);
        QFile::copy(srcName, destName);
    }
}

bool Utilities::shouldCopy(const QString &fileName, const QStringList &include, const QStringList &exclude)
{
    QString fileName2 = QFileInfo(fileName).fileName();

    bool result = false;

    for (int i = 0; i < include.size(); ++i)
    {
        QRegExp rx(include[i]);
        rx.setPatternSyntax(QRegExp::Wildcard);
        if (rx.exactMatch(fileName2))
        {
            result = true;
            break;
        }
    }

    for (int i = 0; i < exclude.size(); ++i)
    {
        QRegExp rx(exclude[i]);
        rx.setPatternSyntax(QRegExp::Wildcard);
        if (rx.exactMatch(fileName2))
        {
            result = false;
            break;
        }
    }

    return result;
}

void Utilities::copyFolder(	const QDir& sourceDir,
                        const QDir& destDir,
                        const QList<QPair<QString, QString> >& renameList,
                        const QList<QStringList>& wildcards,
                        const QList<QList<QPair<QByteArray, QByteArray> > >& replaceList,
                        const QStringList &include,
                        const QStringList &exclude)
{
    if(!sourceDir.exists())
        return;

    QStringList files;

    files = sourceDir.entryList(QDir::Files | QDir::Hidden);
    ExportCommon::progressSteps(files.count());
    for(int i = 0; i < files.count(); i++)
    {
        QString srcName = sourceDir.absoluteFilePath(files[i]);
        QString destFile = files[i];
        for (int i = 0; i < renameList.size(); ++i)
            destFile.replace(renameList[i].first, renameList[i].second);
        QString destName = destDir.absoluteFilePath(destFile);
        if (shouldCopy(srcName, include, exclude))
            fileCopy(srcName, destName, wildcards, replaceList);
        ExportCommon::progressStep(files[i].toUtf8().constData());
    }

    files = sourceDir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
    ExportCommon::progressSteps(files.count());
    for(int i = 0; i < files.count(); i++)
    {
        QDir sourceDir2 = sourceDir;
        bool b1 = sourceDir2.cd(files[i]);

        QDir destDir2 = destDir;
        QString destFile = files[i];
        for (int i = 0; i < renameList.size(); ++i)
            destFile.replace(renameList[i].first, renameList[i].second);
        destDir2.mkdir(destFile);
        bool b2 = destDir2.cd(destFile);

        if (b1 && b2 && shouldCopy(sourceDir2.absolutePath(), QStringList() << "*", exclude))
            copyFolder(sourceDir2,
                       destDir2,
                       renameList,
                       wildcards,
                       replaceList,
                       include,
                       exclude);
        ExportCommon::progressStep(files[i].toUtf8().constData());
    }
}

int Utilities::processOutput(QString command, QString dir, QProcessEnvironment env, bool cmdlog){
    QProcess process;
    if (!dir.isEmpty())
    	process.setWorkingDirectory(dir);
    process.setProcessEnvironment(env);
    process.start(command);
    bool commandOut = !cmdlog;
    while (true)
    {
    	bool end=process.waitForFinished(100);
        QString output = process.readAllStandardOutput();
        if(output.length() > 0){
            if (!commandOut)
            {
            	ExportCommon::exportInfo("%s\n",command.toStdString().c_str());
            	commandOut = true;
            }
        	ExportCommon::exportInfo("%s",output.toStdString().c_str());
        }
        QString error = process.readAllStandardError();
        if(error.length() > 0){
            if(!commandOut)
            	ExportCommon::exportInfo("%s",command.toStdString().c_str());
        	ExportCommon::exportError("%s",error.toStdString().c_str());
        }
    	if ((process.error()!=QProcess::Timedout)||end)
    		break;
    }
    return (process.exitStatus()==QProcess::NormalExit)?process.exitCode():-1;
}
