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
#include <QRegularExpression>

#ifdef Q_OS_MACX
#include <unistd.h>
#endif

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
    Q_UNUSED(rs);
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
            QRegularExpression rx(QRegularExpression::anchoredPattern(QRegularExpression::wildcardToRegularExpression(wildcards[j][i])));
            if (rx.match(srcName2).hasMatch())
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
        QRegularExpression rx(QRegularExpression::anchoredPattern(QRegularExpression::wildcardToRegularExpression(include[i])));
        if (rx.match(fileName2).hasMatch())
        {
            result = true;
            break;
        }
    }

    for (int i = 0; i < exclude.size(); ++i)
    {
        QRegularExpression rx(QRegularExpression::anchoredPattern(QRegularExpression::wildcardToRegularExpression(exclude[i])));
        if (rx.match(fileName2).hasMatch())
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

    files = sourceDir.entryList(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
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

    files = sourceDir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);
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

    QFileInfoList syms = sourceDir.entryInfoList(QDir::Files | QDir::Hidden | QDir::AllDirs | QDir::NoDotAndDotDot);
    for(int i = 0; i < syms.count(); i++)
    {
    	if (syms[i].isSymLink())
    	{
            QString srcName = sourceDir.absoluteFilePath(syms[i].fileName());
            ExportCommon::exportInfo("Processing symlink %s\n",srcName.toStdString().c_str());
            if (shouldCopy(srcName, include, exclude))
            {
                QString destFile = syms[i].fileName();
                for (int i = 0; i < renameList.size(); ++i)
                    destFile.replace(renameList[i].first, renameList[i].second);
                QString destName = destDir.absoluteFilePath(destFile);
                QString target;
#ifdef Q_OS_MACX
                char buffer[1024];
                int linkLen=readlink(srcName.toUtf8().constData(),buffer,1024);
                if (linkLen>=0)
                {
                    buffer[linkLen]=0;
                	target=QString::fromUtf8(buffer);
                }
                else
#else
                	target = syms[i].symLinkTarget();
#endif
                for (int i = 0; i < renameList.size(); ++i)
                    target.replace(renameList[i].first, renameList[i].second);
                ExportCommon::exportInfo("Link %s -> %s\n",destName.toStdString().c_str(),target.toStdString().c_str());
                QFile::remove(destName);
                QFile::link(target,destName);
            }
    	}
    }

}

int Utilities::processOutput(QString command, QStringList args, QString dir, QProcessEnvironment env, bool cmdlog){
    QProcess process;
    if (!dir.isEmpty())
    	process.setWorkingDirectory(dir);
    process.setProcessEnvironment(env);
#ifdef Q_OS_WIN
    if (command=="cmd.exe") //Special case for cmd.exe
        process.setNativeArguments(args.join(' '));
    else
        process.setArguments(args);
#endif
    process.setArguments(args);
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
