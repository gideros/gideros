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

enum ImageTypes
{
  e_noImage,
  e_appIcon,
  e_tvIcon,
  e_splashVertical,
  e_splashHorizontal
};

class ExportXml {
	bool isPlugin;
	QString xmlFile;
	QDomElement exporter;
	QMap<QString,QString> props;
	bool ProcessRules(QDomElement rules);
	bool ProcessRule(QDomElement rule);
	QString ComputeUnary(QString op,QString arg);
	QString ComputeOperator(QString op,QString arg1,QString arg2);
	QString ReplaceAttributes(QString text);
	bool RuleExec(QString cmd,QDomElement rule);
	bool RuleMkdir(QString cmd);
	bool RuleRmdir(QString cmd);
	bool RuleRm(QString cmd);
	bool RuleCd(QString cmd);
	bool RuleCp(QString src,QString dst);
	bool RuleMv(QString src,QString dst);
	bool RuleIf(QString cond,QDomElement rule);
	bool RuleSet(QString key,QString val);
	bool RuleAsk(QDomElement rule);
    bool RuleTemplate(QString name,QString path,QString dest,QDomElement rule);
    bool RuleImage(int width,int height,QString dst, ImageTypes type);
	QString XmlAttributeOrElement(QDomElement elm,QString name);
	ExportContext *ctx;
public:
	ExportXml(QString xmlFile,bool isPlugin=false);
	bool Process(ExportContext *ctx);
	static bool exportXml(QString xmlFile,bool plugin,ExportContext *ctx);
	static QMap<QString, QString> availableTargets();
	static QMap<QString, QString> availablePlugins();
};

#endif /* GDREXPORT_EXPORTXML_H_ */
