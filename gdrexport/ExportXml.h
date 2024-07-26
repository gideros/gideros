/*
 * ExporrtXml.h
 *
 *  Created on: 22 janv. 2016
 *      Author: Nico
 */

#ifndef GDREXPORT_EXPORTXML_H_
#define GDREXPORT_EXPORTXML_H_

#include "exportcontext.h"
#include <QMap>
#include "ExportScript.h"

class ExportXml {
	QDomElement exporter;
	bool ProcessRules(QDomElement rules);
	bool ProcessRule(QDomElement rule);
	QString ComputeUnary(QString op,QString arg);
	QString ComputeOperator(QString op,QString arg1,QString arg2);
	QString ReplaceAttributes(QString text);
	bool RuleExec(QString cmd,QDomElement rule);
	bool RuleIf(QString cond,QDomElement rule);
	bool RuleSet(QString key,QString val);
	bool RuleAsk(QDomElement rule);
    bool RuleTemplate(QString name,QString path,QString dest,QDomElement rule);
	QString XmlAttributeOrElement(QDomElement elm,QString name);
	ExportContext *ctx;
    ExportScript *script;
public:
    ExportXml(ExportScript *script,QString xmlFile,bool isPlugin=false);
    ExportXml(ExportScript *script);
    void SetupProperties(ExportContext *ctx);
	bool Process(ExportContext *ctx);
	bool RunInit(ExportContext *ctx);
	bool ProcessRuleString(const char *xml);
	bool isPlugin;
	QString xmlFile;
	static bool exportXml(QString xmlFile,bool plugin,ExportContext *ctx);
	static bool runinitXml(QString xmlFile,bool plugin,ExportContext *ctx);
};

#endif /* GDREXPORT_EXPORTXML_H_ */
