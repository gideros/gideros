/*
 * ExporrtXml.cpp
 *
 *  Created on: 22 janv. 2016
 *      Author: Nico
 */

#include "ExportXml.h"
#include "Utilities.h"
#include "ExportCommon.h"
#include <QStandardPaths>
#include <QFileInfo>

static bool IsSecret(QString key)
{
	return key.startsWith("secret.");
}

static QString SecretVal(QString val)
{
    Q_UNUSED(val);
	return "******";
}

static QString SecretVal(QString key,QString val)
{
	if (IsSecret(key))
		return SecretVal(val);
	return val;
}

ExportXml::ExportXml(ExportScript *script,QString xmlFile, bool isPlugin) {
	this->isPlugin = isPlugin;
	this->xmlFile=xmlFile;
    this->script=script;
	QDomDocument doc(isPlugin ? "plugin" : "export");
	QFile file(xmlFile);
	if (file.open(QIODevice::ReadOnly)) {
		doc.setContent(&file);
		file.close();
	}

	exporter = doc.documentElement();
}

ExportXml::ExportXml(ExportScript *script) {
    this->isPlugin = true;
    this->xmlFile=NULL;
    this->script=script;
    this->ctx=script->ctx;
}

void ExportXml::SetupProperties(ExportContext *ctx)
{
    script->SetupProperties(ctx);
	this->ctx = ctx;

	//Type specific props
	QString exname = exporter.attribute("name");
	QDir xmlDir=QFileInfo(xmlFile).dir();
	if (isPlugin) {
        script->lprops["sys.pluginDir"] = xmlDir.path();
		//Fill properties: Plugin
		for (QSet<ProjectProperties::Plugin>::const_iterator it =
                ctx->properties.plugins.cbegin();
                it != ctx->properties.plugins.cend(); it++)
			if ((*it).name == exname) {
				for (QMap<QString, QString>::const_iterator mit =
						(*it).properties.begin(); mit != (*it).properties.end();
                        mit++)
                    script->lprops[QString("plugin.").append(mit.key())] = mit.value();
            }
	} else {
//Fill properties: Export
		ctx->props["sys.exporterDir"] = xmlDir.path();
        for (QSet<ProjectProperties::Export>::const_iterator it =
                ctx->properties.exports.cbegin();
                it != ctx->properties.exports.cend(); it++)
            if ((*it).name == exname) {
                for (QMap<QString, QString>::const_iterator mit =
                        (*it).properties.begin(); mit != (*it).properties.end();
                        mit++)
                    ctx->props[QString("export.").append(mit.key())] = mit.value();
            }
    }

}

bool ExportXml::Process(ExportContext *ctx) {
	SetupProperties(ctx);
	QDomElement rules;
	if (isPlugin) {
		//Lookup target
		QDomNodeList targets = exporter.elementsByTagName("target");
		for (int k = 0; k < targets.count(); k++) {
			QString tname=targets.at(k).toElement().attribute("name");
            QStringList tlist=tname.split(',', Qt::SkipEmptyParts);
			if (tlist.contains(ctx->platform))
				rules = targets.at(k).toElement();
		}
	} else {
		rules = exporter.firstChildElement("rules");
    }
//Run rules
	return ProcessRules(rules);
}

bool ExportXml::RunInit(ExportContext *ctx) {
	SetupProperties(ctx);
	QDomElement rules;
	if (isPlugin) {
		//Lookup target
		QDomNode rule = exporter.firstChildElement("initscript");
		if (!rule.isNull())
            return script->OpLua(ReplaceAttributes(rule.toElement().attribute("file")).trimmed(),
	        		rule.toElement().text().trimmed());
	} else {
    }
	return true;
}

bool ExportXml::ProcessRules(QDomElement rules) {
	if (rules.hasChildNodes()) {
		QDomNodeList ruleList = rules.childNodes();
		for (int i = 0; i < ruleList.count(); i++) {
			QDomElement rnode = ruleList.at(i).toElement();
			if (!rnode.isNull())
				if (!ProcessRule(rnode))
					return false;
		}
	}
	return true;
}

bool ExportXml::exportXml(QString xmlFile, bool plugin, ExportContext *ctx) {
    ExportXml *ex = new ExportXml(new ExportScript(),xmlFile, plugin);
	bool ret = ex->Process(ctx);
	delete ex;
	return ret;
}

bool ExportXml::runinitXml(QString xmlFile, bool plugin, ExportContext *ctx) {
    ExportXml *ex = new ExportXml(new ExportScript(),xmlFile, plugin);
	bool ret = ex->RunInit(ctx);
	delete ex;
	return ret;
}


bool ExportXml::ProcessRuleString(const char *xml)
{
	QString input(xml);
	QDomDocument xmlDoc;
	xmlDoc.setContent(input);
	return ProcessRule(xmlDoc.firstChild().toElement());
}

bool ExportXml::ProcessRule(QDomElement rule) {
	QString ruleName = rule.tagName();
	if (ruleName == "exec")
        return RuleExec(ReplaceAttributes(rule.attribute("cmd")).trimmed(), rule);
	else if (ruleName == "set")
		return RuleSet(ReplaceAttributes(rule.attribute("key")),
				ReplaceAttributes(rule.attribute("value")));
	else if (ruleName == "ask")
		return RuleAsk(rule);
	else if (ruleName == "if")
		return RuleIf(ReplaceAttributes(rule.attribute("condition")), rule);
	else if (ruleName == "cp")
        return script->OpCp(ReplaceAttributes(rule.attribute("src")),
				ReplaceAttributes(rule.attribute("dst")));
	else if (ruleName == "mv")
        return script->OpMv(ReplaceAttributes(rule.attribute("src")),
				ReplaceAttributes(rule.attribute("dst")));
	else if (ruleName == "rm")
        return script->OpRm(ReplaceAttributes(rule.text()).trimmed());
	else if (ruleName == "cd")
        return script->OpCd(ReplaceAttributes(rule.text()).trimmed());
	else if (ruleName == "mkdir")
        return script->OpMkdir(ReplaceAttributes(rule.text()).trimmed());
	else if (ruleName == "rmdir")
        return script->OpRmdir(ReplaceAttributes(rule.text()).trimmed());
	else if (ruleName == "download")
		return ExportCommon::download(ctx,
                ReplaceAttributes(rule.attribute("source")).trimmed(),
				ReplaceAttributes(rule.attribute("dest")).trimmed());
	else if (ruleName == "unzip")
		return ExportCommon::unzip(ctx,
                ReplaceAttributes(rule.attribute("source")).trimmed(),
				ReplaceAttributes(rule.attribute("dest")).trimmed());
	else if (ruleName == "template")
		return RuleTemplate(rule.attribute("name"),
                ReplaceAttributes(rule.attribute("path")).trimmed(), ReplaceAttributes(rule.attribute("dest")).trimmed(), rule);
	else if (ruleName == "exportAssets") {
        QStringList jets=rule.attribute("jet").split(";",Qt::SkipEmptyParts);
        QStringList noencExt=rule.attribute("dontEncryptExts").split(";",Qt::SkipEmptyParts);
        script->OpExportAssets(jets,noencExt, rule.attribute("compile").toInt() != 0);
		return true;
	} else if (ruleName == "exportAllfilesTxt") {
        script->OpExportAllFilesTxt();
		return true;
	} else if (ruleName == "exportLuafilesTxt") {
        script->OpExportLuaFilesTxt();
		return true;
	} else if (ruleName == "exportPropertiesBin") {
        script->OpExportPropertiesBin();
		return true;
	} else if (ruleName == "applyPlugins") {
        script->OpApplyPlugins();
		return true;
	} else if (ruleName == "initPlugins") {
        script->OpInitPlugins();
		return true;
	} else if (ruleName == "requestPlugin") {
        script->OpRequestPlugin(rule.attribute("name"));
		return true;
    } else if (ruleName == "appIcon"){
        return script->OpImage(rule.attribute("width").toInt(),
				rule.attribute("height").toInt(),
                ReplaceAttributes(rule.attribute("dest")).trimmed(), e_appIcon,rule.attribute("alpha","1").toInt());
    } else if (ruleName == "tvIcon"){
        return script->OpImage(rule.attribute("width").toInt(),
                rule.attribute("height").toInt(),
                ReplaceAttributes(rule.attribute("dest")).trimmed(), e_tvIcon,rule.attribute("alpha","1").toInt());
    }
    else if (ruleName == "splashVertical"){
        return script->OpImage(rule.attribute("width").toInt(),
                rule.attribute("height").toInt(),
                ReplaceAttributes(rule.attribute("dest")).trimmed(), e_splashVertical,rule.attribute("alpha","1").toInt());
    }
    else if (ruleName == "splashHorizontal"){
        return script->OpImage(rule.attribute("width").toInt(),
                rule.attribute("height").toInt(),
                ReplaceAttributes(rule.attribute("dest")).trimmed(), e_splashHorizontal,rule.attribute("alpha","1").toInt());
    }
    else if (ruleName == "lua"){
        return script->OpLua(ReplaceAttributes(rule.attribute("file")).trimmed(),
	        		rule.text().trimmed());
    }
    else if (ruleName == "openUrl"){
        return script->OpOpenUrl(ReplaceAttributes(rule.text().trimmed()));
    }
	else
		ExportCommon::exportError("Rule %s unknown\n", ruleName.toStdString().c_str());
	return false;
}

QString ExportXml::ComputeUnary(QString op, QString arg) {
	if (op == "bnot")
		return QString::number(~arg.toInt());
	else if (op == "not")
		return QString::number(!arg.toInt());
	else if (op == "exists")
	{
		QFileInfo check_file(arg);
		return check_file.exists()?"1":"0";
	}
	ExportCommon::exportError("Operator '%s' unknown\n", op.toStdString().c_str());
	return "";
}

QString ExportXml::ComputeOperator(QString op, QString arg1, QString arg2) {
	if (op == "eq")
		return (arg1 == arg2) ? "1" : "0";
	else if (op == "neq")
		return (arg1 != arg2) ? "1" : "0";
	else if (op == "lt")
		return (arg1 < arg2) ? "1" : "0";
	else if (op == "lte")
		return (arg1 <= arg2) ? "1" : "0";
	else if (op == "gt")
		return (arg1 > arg2) ? "1" : "0";
	else if (op == "gte")
		return (arg1 >= arg2) ? "1" : "0";
	else if (op == "add")
		return QString::number(arg1.toInt() + arg2.toInt());
	else if (op == "sub")
		return QString::number(arg1.toInt() - arg2.toInt());
	else if (op == "div")
		return QString::number(arg1.toInt() * arg2.toInt());
	else if (op == "mul")
		return QString::number(arg1.toInt() / arg2.toInt());
	else if (op == "mod")
		return QString::number(arg1.toInt() % arg2.toInt());
	else if (op == "band")
		return QString::number(arg1.toInt() & arg2.toInt());
	else if (op == "and")
		return QString::number(arg1.toInt() && arg2.toInt());
	else if (op == "bor")
		return QString::number(arg1.toInt() | arg2.toInt());
	else if (op == "or")
		return QString::number(arg1.toInt() || arg2.toInt());
	else if (op == "bxor")
		return QString::number(arg1.toInt() ^ arg2.toInt());
	ExportCommon::exportError("Operator '%s' unknown\n", op.toStdString().c_str());
	return "";
}

QString ExportXml::ReplaceAttributes(QString text) {
	int epos = -1;
	bool secret=false;
	while ((epos = text.indexOf("]]]")) != -1) {
		int spos = text.lastIndexOf("[[[", epos);
		if (spos == -1)
			break;
		QString key = text.mid(spos + 3, epos - spos - 3);
		secret|=IsSecret(key);
        QStringList args = key.split(":", Qt::KeepEmptyParts);
		int ac = args.count();
		QString rep;
		if (ac == 1)
            rep = script->GetProperty(args[0]);
		else if (ac == 2)
			rep = ComputeUnary(args[0], args[1]);
		else if (ac == 3)
			rep = ComputeOperator(args[0], args[1], args[2]);
		text = text.replace(spos, epos + 3 - spos, rep);
		ExportCommon::exportInfo("Replaced %s by %s @%d\n", (((ac>=2)&&secret)?SecretVal(key):key).toStdString().c_str(),
				(secret?SecretVal(rep):rep).toStdString().c_str(), spos);
	}
	return text;
}

bool ExportXml::RuleExec(QString cmd, QDomElement rule) {
	QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QStringList args;
	for (QDomNode n = rule.firstChild(); !n.isNull(); n = n.nextSibling()) {
		QDomElement rl = n.toElement();
		if ((!rl.isNull()) && (rl.tagName() == "env"))
			env.insert(rl.attribute("key"),
					ReplaceAttributes(rl.attribute("value")));
        else if ((!rl.isNull()) && (rl.tagName() == "arg"))
            args << ReplaceAttributes(rl.text().trimmed());
        else if ((!rl.isNull()) && (rl.tagName() == "oarg")) {
            QString arg=ReplaceAttributes(rl.text().trimmed());
            if (arg.length()>0)
                args << arg;
        }
    }
    return script->OpExec(cmd,args,env);
}


bool ExportXml::RuleIf(QString cond, QDomElement rule) {
	ExportCommon::exportInfo("If: %s\n", cond.toStdString().c_str());
	if (cond.toInt())
		return ProcessRules(rule);
	return true;
}

bool ExportXml::RuleSet(QString key, QString val) {
	ExportCommon::exportInfo("Set: %s -> %s\n", key.toStdString().c_str(),
			SecretVal(key,val).toStdString().c_str());
    script->SetProperty(key,val);
	return true;
}

bool ExportXml::RuleAsk(QDomElement rule) {
	QString key=XmlAttributeOrElement(rule,"key");
	QString title=ReplaceAttributes(XmlAttributeOrElement(rule,"title"));
	QString question=ReplaceAttributes(XmlAttributeOrElement(rule,"question"));
	QString def=ReplaceAttributes(XmlAttributeOrElement(rule,"default"));
	QString uid=ReplaceAttributes(XmlAttributeOrElement(rule,"uid"));
    return script->OpAsk(key,title,question,def,uid);
}


QString ExportXml::XmlAttributeOrElement(QDomElement elm,QString name)
{
	QString value=elm.attribute(name);
	if (!value.isEmpty())
		return value;
	QDomNodeList nl=elm.elementsByTagName(name);
	for (int i=0;i<nl.count();i++)
	{
		QDomElement se=nl.item(i).toElement();
		value=value+se.text();
	}
	return value;
}

bool ExportXml::RuleTemplate(QString name, QString path, QString dest, QDomElement rule) {
    QStringList include= ReplaceAttributes(rule.attribute("include")).split(";",Qt::SkipEmptyParts);
    QStringList exclude= ReplaceAttributes(rule.attribute("exclude")).split(";",Qt::SkipEmptyParts);
	for (QDomNode n = rule.firstChild(); !n.isNull(); n = n.nextSibling()) {
		QDomElement rl = n.toElement();
		if ((!rl.isNull()) && (rl.tagName() == "replacelist")) {
            QStringList wildcards1 = ReplaceAttributes(rl.attribute("wildcards")).split(";",
                    Qt::SkipEmptyParts);
			QList < QPair<QByteArray, QByteArray> > replaceList1;
			for (QDomNode n1 = rl.firstChild(); !n1.isNull();
					n1 = n1.nextSibling()) {
				QDomElement rp = n1.toElement();
                if ((!rp.isNull())){
                	QString orig=ReplaceAttributes(XmlAttributeOrElement(rp,"orig"));
                	QString by=ReplaceAttributes(XmlAttributeOrElement(rp,"by"));
                	bool force=(rp.tagName() == "replace"); //Force by default on replace, not on append/prepend
                    if(rp.attribute("force") == "true")
                    	force=true;
                    if(rp.attribute("force") == "false")
                    	force=false;
                    if (!force)
                        replaceList1
                                << qMakePair(
                                        by.toUtf8(),
                                        QString("").toUtf8());
                    if ((rp.tagName() == "replace"))
                        replaceList1
                                << qMakePair(
                                        orig.toUtf8(),
                                        by.toUtf8());
                    else if ((!rp.isNull()) && (rp.tagName() == "prepend"))
                            replaceList1
                                    << qMakePair(
                                            orig.toUtf8(),
                                            by.toUtf8()+"\n"+orig.toUtf8());
                    else if ((!rp.isNull()) && (rp.tagName() == "append"))
                            replaceList1
                                    << qMakePair(
                                            orig.toUtf8(),
                                            orig.toUtf8()+"\n"+by.toUtf8());
                }
            }

			ctx->wildcards << wildcards1;
			ctx->replaceList << replaceList1;
		}
	}

	ctx->templatename = name;
    ctx->templatenamews = Utilities::RemoveSpaces(name, Utilities::NODIGIT); //TODO underscores or not ?
    ExportCommon::exportInfo("Template: %s from [%s] to [%s]\n", name.toStdString().c_str(),
            path.toStdString().c_str(), dest.toStdString().c_str());

	ExportCommon::copyTemplate(
			QDir::current().relativeFilePath(
                    ctx->outputDir.absoluteFilePath(path)), ctx->outputDir.absoluteFilePath(dest), ctx, isPlugin, include, exclude);
	return true;
}

