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
	fprintf(stderr, "Rule: %s\n",ruleName.toStdString().c_str());
	if (ruleName=="exec")
		return RuleExec(ReplaceAttributes(rule.text()).trimmed());
	else if (ruleName=="cd")
		return RuleCd(ReplaceAttributes(rule.text()).trimmed());
	else if (ruleName=="mkdir")
		return RuleMkdir(ReplaceAttributes(rule.text()).trimmed());
	else if (ruleName=="rmdir")
		return RuleRmdir(ReplaceAttributes(rule.text()).trimmed());
	else if (ruleName=="template")
		return RuleTemplate(rule.attribute("name"),ReplaceAttributes(rule.attribute("path")).trimmed());
	else if (ruleName=="exportAssets")
	{ ExportCommon::exportAssets(ctx,rule.attribute("compile").toInt()!=0); return true; }
	else if (ruleName=="exportAllfilesTxt")
	{ ExportCommon::exportAllfilesTxt(ctx); return true; }
	else if (ruleName=="exportLuafilesTxt")
	{ ExportCommon::exportLuafilesTxt(ctx); return true; }
	else if (ruleName=="exportPropertiesBin")
	{ ExportCommon::exportPropertiesBin(ctx); return true; }
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

bool ExportXml::RuleExec(QString cmd)
{
    fprintf(stderr, "Exec: %s into %s\n",cmd.toStdString().c_str(),ctx->outputDir.path().toStdString().c_str());
	int err=Utilities::processOutput(cmd,ctx->outputDir.path());
	return (err==0);
}

bool ExportXml::RuleMkdir(QString cmd)
{
	 return ctx->outputDir.mkpath(cmd);

}

bool ExportXml::RuleRmdir(QString cmd)
{
	if (!ctx->outputDir.cd(cmd))
		return false;
    if (!ctx->outputDir.removeRecursively())
    	return false;
    if (!ctx->outputDir.cdUp())
    	return false;
    return true;
}

bool ExportXml::RuleCd(QString cmd)
{
 return ctx->outputDir.cd(cmd);
}

bool ExportXml::RuleTemplate(QString name,QString path)
{
	ctx->templatename = name;
	ctx->templatenamews = Utilities::RemoveSpaces(name,false); //TODO underscores or not ?
	ctx->basews=Utilities::RemoveSpaces(ctx->base,false);
	ExportCommon::copyTemplate(QDir::current().relativeFilePath(ctx->outputDir.absoluteFilePath(path)),ctx);
	return true;
}
