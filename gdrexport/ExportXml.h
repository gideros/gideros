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

class ExportXml {
	QDomElement exporter;
	QMap<QString,QString> props;
	bool ProcessRule(QDomElement rule);
	QString ReplaceAttributes(QString text);
	bool RuleExec(QString cmd,QDomElement rule);
	bool RuleMkdir(QString cmd);
	bool RuleRmdir(QString cmd);
	bool RuleRm(QString cmd);
	bool RuleCd(QString cmd);
	bool RuleCp(QString src,QString dst);
	bool RuleMv(QString src,QString dst);
	bool RuleTemplate(QString name,QString path,QDomElement rule);
	ExportContext *ctx;
public:
	ExportXml(QString xmlFile);
	void Process(ExportContext *ctx);
	static void exportXml(QString xmlFile,ExportContext *ctx);
	static QMap<QString, QString> availableTargets();
};

#endif /* GDREXPORT_EXPORTXML_H_ */
