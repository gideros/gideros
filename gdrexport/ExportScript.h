/*
 * ExporrtXml.h
 *
 *  Created on: 22 janv. 2016
 *      Author: Nico
 */

#ifndef GDREXPORT_EXPORTSCRIPT_H_
#define GDREXPORT_EXPORTSCRIPT_H_

#include "exportcontext.h"
#include <QMap>
#include <QStringList>
#include <QDir>
#include <QProcess>

enum ImageTypes
{
  e_noImage,
  e_appIcon,
  e_tvIcon,
  e_splashVertical,
  e_splashHorizontal
};

class ExportScript {
public:
    typedef struct TemplateReplacement {
        QString original;
        QString replacement;
        enum ReplacementMode {
            REPLACE=0,
            APPEND=1,
            PREPEND=2
        } mode;
        bool force;
    } TemplateReplacement;
    typedef struct TemplateReplacements {
        QStringList wildcards;
        QList<TemplateReplacement> replacements;
        bool force;
    } TemplateReplacements;
    bool OpExec(QString cmd, QStringList args, QProcessEnvironment env);
    bool OpMkdir(QString cmd);
    bool OpRmdir(QString cmd);
    bool OpRm(QString cmd);
    bool OpCd(QString cmd);
    bool OpCp(QString src,QString dst);
    bool OpMv(QString src,QString dst);
    bool OpAsk(QString key,QString title,QString question,QString def,QString uid);
    bool OpTemplate(QString name, QString path, QString dest, QStringList include, QStringList exclude, QList<TemplateReplacements> replacementsList, bool addon);
    bool OpImage(int width,int height,QString dst, ImageTypes type, bool alpha);
    bool OpLua(QString file,QString content);
    bool OpDownload(QString source,QString dest);
    bool OpUnzip(QString source,QString dest);
    bool OpOpenUrl(QString url);
    void OpExportAssets(QStringList jets,QStringList noencExt,bool compile);
    void OpExportAllFilesTxt();
    void OpExportLuaFilesTxt();
    void OpExportPropertiesBin();
    void OpApplyPlugins();
    void OpInitPlugins();
    void OpRequestPlugin(QString name);

	ExportContext *ctx;
public:
    ExportScript();
	void SetProperty(QString k,QString v);
	QString GetProperty(QString k);
	void SetupProperties(ExportContext *ctx);
	QMap<QString,QString> lprops;
	static QMap<QString, QString> availableTargets();
	static QMap<QString, QString> availablePlugins();
};

#endif /* GDREXPORT_EXPORTXML_H_ */
