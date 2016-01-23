/*
 * ExporrtXml.cpp
 *
 *  Created on: 22 janv. 2016
 *      Author: Nico
 */

#include "ExportXml.h"
#include "Utilities.h"
#include "ExportCommon.h"

ExportXml::ExportXml(QString xmlFile) {
	QDomDocument doc("export");
	QFile file(xmlFile);
	if (file.open(QIODevice::ReadOnly))
	{
		doc.setContent(&file);
		file.close();
	}

	exporter = doc.documentElement();
}

void ExportXml::Process(ExportContext *ctx) {
	this->ctx=ctx;
	ctx->basews=Utilities::RemoveSpaces(ctx->base,false);
	QString exname = exporter.attribute("name");
 //Fill properties: System
#ifdef Q_OS_WIN32
	props["sys.exeExtension"]=".exe";
#else
	props["sys.exeExtension"]="";
#endif
	props["sys.giderosDir"]=QDir::currentPath();
//Fill properties: Export
   	for (QSet<ProjectProperties::Export>::const_iterator it=ctx->properties.exports.begin();it!=ctx->properties.exports.end(); it++)
    		if ((*it).name==exname)
    		{
    		   	for (QMap<QString,QString>::const_iterator mit=(*it).properties.begin();mit!=(*it).properties.end(); mit++)
    		   		props[QString("exprop.").append(mit.key())]=mit.value();
    		}
//Fill properties: Project
   	props["project.name"]=ctx->base;
   	props["project.namews"]=ctx->basews;
   	props["project.package"]=ctx->properties.packageName;
   	props["project.version"]=ctx->properties.version;
   	props["project.version_code"]=QString::number(ctx->properties.version_code);
//Run rules
   	QDomElement rules=exporter.firstChildElement("rules");
   	if (rules.hasChildNodes() )
   	{
   		QDomNodeList ruleList = rules.childNodes();
   		for (int i=0;i<ruleList.count();i++)
   		{
   			QDomElement rnode=ruleList.at(i).toElement();
   			if (!rnode.isNull())
   				if (!ProcessRule(rnode)) return;
   		}
   	}
}

void ExportXml::exportXml(QString xmlFile, ExportContext *ctx) {
	ExportXml *ex = new ExportXml(xmlFile);
	ex->Process(ctx);
	delete ex;
}

QMap<QString, QString> ExportXml::availableTargets() {
	QMap < QString, QString > xmlExports;
	QDir sourceDir("Templates");
	QStringList filters;
	filters << "*.gexport";
	sourceDir.setNameFilters(filters);
	QStringList files = sourceDir.entryList(QDir::Files | QDir::Hidden);
	for (int i = 0; i < files.count(); i++) {
		QDomDocument doc("export");
		QFile file(sourceDir.absoluteFilePath(files[i]));
		if (!file.open(QIODevice::ReadOnly))
			continue;
		if (!doc.setContent(&file)) {
			file.close();
			continue;
		}
		file.close();

		QDomElement exporter = doc.documentElement();
		QString exname = exporter.attribute("name");
		xmlExports[exname] = sourceDir.absoluteFilePath(files[i]);
	}
	return xmlExports;
}

bool ExportXml::ProcessRule(QDomElement rule)
{
	QString ruleName=rule.tagName();
	if (ruleName=="exec")
		return RuleExec(ReplaceAttributes(rule.text()).trimmed(),rule);
	else if (ruleName=="cp")
		return RuleCp(ReplaceAttributes(rule.attribute("src")),ReplaceAttributes(rule.attribute("dst")));
	else if (ruleName=="mv")
		return RuleMv(ReplaceAttributes(rule.attribute("src")),ReplaceAttributes(rule.attribute("dst")));
	else if (ruleName=="rm")
		return RuleRm(ReplaceAttributes(rule.text()).trimmed());
	else if (ruleName=="cd")
		return RuleCd(ReplaceAttributes(rule.text()).trimmed());
	else if (ruleName=="mkdir")
		return RuleMkdir(ReplaceAttributes(rule.text()).trimmed());
	else if (ruleName=="rmdir")
		return RuleRmdir(ReplaceAttributes(rule.text()).trimmed());
	else if (ruleName=="template")
		return RuleTemplate(rule.attribute("name"),ReplaceAttributes(rule.attribute("path")).trimmed(),rule);
	else if (ruleName=="exportAssets")
	{ ExportCommon::exportAssets(ctx,rule.attribute("compile").toInt()!=0); return true; }
	else if (ruleName=="exportAllfilesTxt")
	{ ExportCommon::exportAllfilesTxt(ctx); return true; }
	else if (ruleName=="exportLuafilesTxt")
	{ ExportCommon::exportLuafilesTxt(ctx); return true; }
	else if (ruleName=="exportPropertiesBin")
	{ ExportCommon::exportPropertiesBin(ctx); return true; }
	else if (ruleName=="appIcon")
		return RuleAppIcon(rule.attribute("width").toInt(),rule.attribute("height").toInt(),ReplaceAttributes(rule.attribute("dest")).trimmed());
	else
	  fprintf(stderr, "Rule %s unknown\n",ruleName.toStdString().c_str());
	return false;
}

QString ExportXml::ReplaceAttributes(QString text)
{
	QRegExp rx("(\\[\\[\\[[^\\]]+\\]\\]\\])");
	int pos = 0;

	while ((pos = rx.indexIn(text, pos)) != -1) {
		QString key=text.mid(pos+3,rx.matchedLength()-6);
		QString rep=props[key];
		text=text.replace(pos,rx.matchedLength(),rep);
        fprintf(stderr, "Replaced %s by %s @%d\n",key.toStdString().c_str(),rep.toStdString().c_str(),pos);
	    pos += rep.length();
	}
	return text;
}

bool ExportXml::RuleExec(QString cmd, QDomElement rule)
{
	QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
	for(QDomNode n = rule.firstChild(); !n.isNull(); n = n.nextSibling())
	{
		QDomElement rl = n.toElement();
		if ((!rl.isNull())&&(rl.tagName()=="env"))
			env.insert(rl.attribute("key"),ReplaceAttributes(rl.attribute("value")));
	}
    fprintf(stderr, "Exec: %s into %s\n",cmd.toStdString().c_str(),ctx->outputDir.path().toStdString().c_str());
	int err=Utilities::processOutput(cmd,ctx->outputDir.path(),env);
    fprintf(stderr, "Exec returned: %d\n",err);
	return (err==0);
}

bool ExportXml::RuleMkdir(QString cmd)
{
    fprintf(stderr, "MkDir: %s\n",cmd.toStdString().c_str());
	 return ctx->outputDir.mkpath(cmd);
}

bool ExportXml::RuleRmdir(QString cmd)
{
    fprintf(stderr, "RmDir: %s\n",cmd.toStdString().c_str());
    QDir remdir=ctx->outputDir;
	if (!remdir.cd(cmd))
		return false;
    if (!remdir.removeRecursively())
    	return false;
    return true;
}

bool ExportXml::RuleCd(QString cmd)
{
 fprintf(stderr, "Cd: %s\n",cmd.toStdString().c_str());
 return ctx->outputDir.cd(cmd);
}

bool ExportXml::RuleRm(QString cmd)
{
 fprintf(stderr, "Rm: %s\n",cmd.toStdString().c_str());
 ctx->outputDir.remove(cmd);
 return !ctx->outputDir.exists(cmd);
}

bool ExportXml::RuleCp(QString src,QString dst)
{
    fprintf(stderr, "Cp: %s -> %s\n",src.toStdString().c_str(),dst.toStdString().c_str());
    ctx->outputDir.remove(dst);
	return QFile::copy(ctx->outputDir.absoluteFilePath(src),ctx->outputDir.absoluteFilePath(dst));
}

bool ExportXml::RuleMv(QString src,QString dst)
{
    fprintf(stderr, "Mv: %s -> %s\n",src.toStdString().c_str(),dst.toStdString().c_str());
	 return ctx->outputDir.rename(src,dst);
}

bool ExportXml::RuleTemplate(QString name,QString path,QDomElement rule)
{
	for(QDomNode n = rule.firstChild(); !n.isNull(); n = n.nextSibling())
	{
		QDomElement rl = n.toElement();
		if ((!rl.isNull())&&(rl.tagName()=="replacelist"))
		{
		    QStringList wildcards1=rl.attribute("wildcards").split(";", QString::SkipEmptyParts);
		    QList<QPair<QByteArray, QByteArray> > replaceList1;
			for(QDomNode n1 = rl.firstChild(); !n1.isNull(); n1 = n1.nextSibling())
			{
				QDomElement rp = n1.toElement();
				if ((!rp.isNull())&&(rp.tagName()=="replace"))
			        replaceList1 << qMakePair(ReplaceAttributes(rp.attribute("orig")).toUtf8(), ReplaceAttributes(rp.attribute("by")).toUtf8());
			}
		    ctx->wildcards << wildcards1;
	        ctx->replaceList << replaceList1;
		}
	}

	ctx->templatename = name;
	ctx->templatenamews = Utilities::RemoveSpaces(name,false); //TODO underscores or not ?
    fprintf(stderr, "Template: %s [%s]\n",name.toStdString().c_str(),path.toStdString().c_str());
	ExportCommon::copyTemplate(QDir::current().relativeFilePath(ctx->outputDir.absoluteFilePath(path)),ctx);
	return true;
}

bool ExportXml::RuleAppIcon(int width,int height,QString dst)
{
 fprintf(stderr, "AppIcon: %dx%d %s\n",width,height,dst.toStdString().c_str());
 return ExportCommon::appIcon(ctx,width,height,dst);
}
