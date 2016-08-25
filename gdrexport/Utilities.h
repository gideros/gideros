/*
 * Utilities.h
 *
 *  Created on: 22 janv. 2016
 *      Author: Nico
 */

#ifndef GDREXPORT_UTILITIES_H_
#define GDREXPORT_UTILITIES_H_

#include <QString>
#include <QList>
#include <QStringList>
#include <QDir>
#include <QProcess>

class Utilities {
public:
	static QString RemoveSpaces(QString text,bool allowUnderscore);
	static bool bitwiseMatchReplace(unsigned char *b,int bo,const unsigned char *m,int ms,const unsigned char *r);
	static int bitwiseReplace(char *b,int bs,const char *m,int ms,const char *r,int rs);
	static void fileCopy(	const QString& srcName,
	                        const QString& destName,
	                        const QList<QStringList>& wildcards,
	                        const QList<QList<QPair<QByteArray, QByteArray> > >& replaceList);
	static bool shouldCopy(const QString &fileName, const QStringList &include, const QStringList &exclude);
	static void copyFolder(	const QDir& sourceDir,
		                        const QDir& destDir,
		                        const QList<QPair<QString, QString> >& renameList,
		                        const QList<QStringList>& wildcards,
		                        const QList<QList<QPair<QByteArray, QByteArray> > >& replaceList,
		                        const QStringList &include,
		                        const QStringList &exclude);
	static int processOutput(QString command, QString dir=QString(),QProcessEnvironment env=QProcessEnvironment::systemEnvironment(),bool cmdLog=true);
};

#endif /* GDREXPORT_UTILITIES_H_ */
