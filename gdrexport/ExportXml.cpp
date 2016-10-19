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

#ifdef Q_OS_MACX
#define ALL_PLUGINS_PATH "../../All Plugins"
#else
#define ALL_PLUGINS_PATH "All Plugins"
#endif

ExportXml::ExportXml(QString xmlFile, bool isPlugin) {
	this->isPlugin = isPlugin;
	this->xmlFile=xmlFile;
	QDomDocument doc(isPlugin ? "plugin" : "export");
	QFile file(xmlFile);
	if (file.open(QIODevice::ReadOnly)) {
		doc.setContent(&file);
		file.close();
	}

	exporter = doc.documentElement();
}

bool ExportXml::Process(ExportContext *ctx) {
	this->ctx = ctx;
	ctx->basews = Utilities::RemoveSpaces(ctx->base, false);
	QString exname = exporter.attribute("name");
	//Fill properties: System
#ifdef Q_OS_WIN32
	props["sys.exeExtension"]=".exe";
#else
	props["sys.exeExtension"] = "";
#endif
	props["sys.cacheDir"] = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
	props["sys.giderosDir"] = QDir::currentPath();
    props["sys.exportDir"] = ctx->exportDir.absolutePath();
	QDomElement rules;
	QDir xmlDir=QFileInfo(xmlFile).dir();
	if (isPlugin) {
		props["sys.pluginDir"] = xmlDir.path();
		//Fill properties: Plugin
		for (QSet<ProjectProperties::Plugin>::const_iterator it =
				ctx->properties.plugins.begin();
                it != ctx->properties.plugins.end(); it++)
			if ((*it).name == exname) {
				for (QMap<QString, QString>::const_iterator mit =
						(*it).properties.begin(); mit != (*it).properties.end();
                        mit++)
                    props[QString("plugin.").append(mit.key())] = mit.value();
            }
		//Lookup target
		QDomNodeList targets = exporter.elementsByTagName("target");
		QStringList targetList;
		for (int k = 0; k < targets.count(); k++) {
			QString tname=targets.at(k).toElement().attribute("name");
			QStringList tlist=tname.split(',', QString::SkipEmptyParts);
			if (tlist.contains(ctx->platform))
				rules = targets.at(k).toElement();
		}
	} else {
//Fill properties: Export
		props["sys.exportDir"] = xmlDir.path();
		rules = exporter.firstChildElement("rules");
        for (QSet<ProjectProperties::Export>::const_iterator it =
                ctx->properties.exports.begin();
                it != ctx->properties.exports.end(); it++)
            if ((*it).name == exname) {
                for (QMap<QString, QString>::const_iterator mit =
                        (*it).properties.begin(); mit != (*it).properties.end();
                        mit++)
                    props[QString("export.").append(mit.key())] = mit.value();
            }
    }
//Fill properties: Project
	props["project.name"] = ctx->base;
	props["project.namews"] = ctx->basews;
	props["project.package"] = ctx->properties.packageName;
	props["project.version"] = ctx->properties.version;
	props["project.version_code"] = QString::number(
			ctx->properties.version_code);
	props["project.autorotation"] = QString::number(
			ctx->properties.autorotation);
	props["project.orientation"] = QString::number(ctx->properties.orientation);
	props["project.disableSplash"] = QString::number(ctx->properties.disableSplash?1:0);
	props["project.backgroundColor"] = ctx->properties.backgroundColor;

//Fill in passed arguments
    QHash<QString, QString>::iterator i;
        for (i = ctx->args.begin(); i != ctx->args.end(); ++i)
            props["args."+i.key()] = i.value();
//Run rules
	return ProcessRules(rules);
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
	ExportXml *ex = new ExportXml(xmlFile, plugin);
	bool ret = ex->Process(ctx);
	delete ex;
	return ret;
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

QMap<QString, QString> ExportXml::availablePlugins() {
	QMap < QString, QString > xmlPlugins;
	QStringList plugins;
	QStringList dirs;
	QDir sourceDir(ALL_PLUGINS_PATH);
	dirs = sourceDir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
	for (int i = 0; i < dirs.count(); i++) {
		QDir sourceDir2 = sourceDir;
		if (sourceDir2.cd(dirs[i])) {
			QStringList filters;
			filters << "*.gplugin";
			sourceDir2.setNameFilters(filters);
			QStringList files = sourceDir2.entryList(
					QDir::Files | QDir::Hidden);
			for (int i = 0; i < files.count(); i++)
				plugins << sourceDir2.absoluteFilePath(files[i]);
		}
	}

	for (int i = 0; i < plugins.count(); i++) {
		QDomDocument doc("plugin");
		QFile file(plugins[i]);
		if (!file.open(QIODevice::ReadOnly))
			continue;
		if (!doc.setContent(&file)) {
			file.close();
			continue;
		}
		file.close();
		QDomElement exporter = doc.documentElement();
		QString exname = exporter.attribute("name");
		xmlPlugins[exname] = plugins[i];
	}
	return xmlPlugins;
}

bool ExportXml::ProcessRule(QDomElement rule) {
	QString ruleName = rule.tagName();
	if (ruleName == "exec")
		return RuleExec(ReplaceAttributes(rule.text()).trimmed(), rule);
	else if (ruleName == "set")
		return RuleSet(ReplaceAttributes(rule.attribute("key")),
				ReplaceAttributes(rule.attribute("value")));
	else if (ruleName == "ask")
		return RuleAsk(rule);
	else if (ruleName == "if")
		return RuleIf(ReplaceAttributes(rule.attribute("condition")), rule);
	else if (ruleName == "cp")
		return RuleCp(ReplaceAttributes(rule.attribute("src")),
				ReplaceAttributes(rule.attribute("dst")));
	else if (ruleName == "mv")
		return RuleMv(ReplaceAttributes(rule.attribute("src")),
				ReplaceAttributes(rule.attribute("dst")));
	else if (ruleName == "rm")
		return RuleRm(ReplaceAttributes(rule.text()).trimmed());
	else if (ruleName == "cd")
		return RuleCd(ReplaceAttributes(rule.text()).trimmed());
	else if (ruleName == "mkdir")
		return RuleMkdir(ReplaceAttributes(rule.text()).trimmed());
	else if (ruleName == "rmdir")
		return RuleRmdir(ReplaceAttributes(rule.text()).trimmed());
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
		QStringList jets=rule.attribute("jet").split(";",QString::SkipEmptyParts);
		for (int i=0;i<jets.count();i++)
			ctx->jetset << jets[i];
		ExportCommon::exportAssets(ctx, rule.attribute("compile").toInt() != 0);
		return true;
	} else if (ruleName == "exportAllfilesTxt") {
		ExportCommon::exportAllfilesTxt(ctx);
		return true;
	} else if (ruleName == "exportLuafilesTxt") {
		ExportCommon::exportLuafilesTxt(ctx);
		return true;
	} else if (ruleName == "exportPropertiesBin") {
		ExportCommon::exportPropertiesBin(ctx);
		return true;
	} else if (ruleName == "applyPlugins") {
		ExportCommon::applyPlugins(ctx);
		return true;
    } else if (ruleName == "appIcon"){
        return RuleImage(rule.attribute("width").toInt(),
				rule.attribute("height").toInt(),
                ReplaceAttributes(rule.attribute("dest")).trimmed(), e_appIcon);
    } else if (ruleName == "tvIcon"){
        return RuleImage(rule.attribute("width").toInt(),
                rule.attribute("height").toInt(),
                ReplaceAttributes(rule.attribute("dest")).trimmed(), e_tvIcon);
    }
    else if (ruleName == "splashVertical"){
        return RuleImage(rule.attribute("width").toInt(),
                rule.attribute("height").toInt(),
                ReplaceAttributes(rule.attribute("dest")).trimmed(), e_splashVertical);
    }
    else if (ruleName == "splashHorizontal"){
        return RuleImage(rule.attribute("width").toInt(),
                rule.attribute("height").toInt(),
                ReplaceAttributes(rule.attribute("dest")).trimmed(), e_splashHorizontal);
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
	while ((epos = text.indexOf("]]]")) != -1) {
		int spos = text.lastIndexOf("[[[", epos);
		if (spos == -1)
			break;
		QString key = text.mid(spos + 3, epos - spos - 3);
		QStringList args = key.split(":", QString::KeepEmptyParts);
		int ac = args.count();
		QString rep;
		if (ac == 1)
			rep = props[args[0]];
		else if (ac == 2)
			rep = ComputeUnary(args[0], args[1]);
		else if (ac == 3)
			rep = ComputeOperator(args[0], args[1], args[2]);
		text = text.replace(spos, epos + 3 - spos, rep);
		ExportCommon::exportInfo("Replaced %s by %s @%d\n", key.toStdString().c_str(),
				rep.toStdString().c_str(), spos);
	}
	return text;
}

bool ExportXml::RuleExec(QString cmd, QDomElement rule) {
	QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
	for (QDomNode n = rule.firstChild(); !n.isNull(); n = n.nextSibling()) {
		QDomElement rl = n.toElement();
		if ((!rl.isNull()) && (rl.tagName() == "env"))
			env.insert(rl.attribute("key"),
					ReplaceAttributes(rl.attribute("value")));
	}
	ExportCommon::exportInfo("Exec: %s into %s\n", cmd.toStdString().c_str(),
			ctx->outputDir.path().toStdString().c_str());
	int err = Utilities::processOutput(cmd, ctx->outputDir.path(), env,false);
	ExportCommon::exportInfo("Exec returned: %d\n", err);
	return (err == 0);
}

bool ExportXml::RuleMkdir(QString cmd) {
	ExportCommon::exportInfo("MkDir: %s\n", cmd.toStdString().c_str());
	return ctx->outputDir.mkpath(cmd);
}

bool ExportXml::RuleRmdir(QString cmd) {
	ExportCommon::exportInfo("RmDir: %s\n", cmd.toStdString().c_str());
	QDir remdir = ctx->outputDir;
	if (!remdir.exists(cmd))
		return true;
	if (!remdir.cd(cmd))
		return false;
	if (!remdir.removeRecursively())
		return false;
	return true;
}

bool ExportXml::RuleCd(QString cmd) {
	ExportCommon::exportInfo("Cd: %s\n", cmd.toStdString().c_str());
	return ctx->outputDir.cd(cmd);
}

bool ExportXml::RuleRm(QString cmd) {
	ExportCommon::exportInfo("Rm: %s\n", cmd.toStdString().c_str());
	ctx->outputDir.remove(cmd);
	return !ctx->outputDir.exists(cmd);
}

bool ExportXml::RuleCp(QString src, QString dst) {
	ExportCommon::exportInfo("Cp: %s -> %s\n", src.toStdString().c_str(),
			dst.toStdString().c_str());
	ctx->outputDir.remove(dst);
	return QFile::copy(ctx->outputDir.absoluteFilePath(src),
			ctx->outputDir.absoluteFilePath(dst));
}

bool ExportXml::RuleMv(QString src, QString dst) {
	ExportCommon::exportInfo("Mv: %s -> %s\n", src.toStdString().c_str(),
			dst.toStdString().c_str());
	return ctx->outputDir.rename(src, dst);
}

bool ExportXml::RuleIf(QString cond, QDomElement rule) {
	ExportCommon::exportInfo("If: %s\n", cond.toStdString().c_str());
	if (cond.toInt())
		return ProcessRules(rule);
	return true;
}

bool ExportXml::RuleSet(QString key, QString val) {
	ExportCommon::exportInfo("Set: %s -> %s\n", key.toStdString().c_str(),
			val.toStdString().c_str());
	props[key] = val;
	return true;
}

bool ExportXml::RuleAsk(QDomElement rule) {
	QString key=XmlAttributeOrElement(rule,"key");
	QString title=ReplaceAttributes(XmlAttributeOrElement(rule,"title"));
	QString question=ReplaceAttributes(XmlAttributeOrElement(rule,"question"));
	QString def=ReplaceAttributes(XmlAttributeOrElement(rule,"default"));
	char *ret=ExportCommon::askString(title.toUtf8().data(),question.toUtf8().data(),def.toUtf8().data());
	QString val=QString::fromUtf8(ret);
	free(ret);
	ExportCommon::exportInfo("Ask: %s -> %s\n", key.toStdString().c_str(),
			val.toStdString().c_str());
	props[key] = val;
	return true;
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
	QStringList include= ReplaceAttributes(rule.attribute("include")).split(";",QString::SkipEmptyParts);
	QStringList exclude= ReplaceAttributes(rule.attribute("exclude")).split(";",QString::SkipEmptyParts);
	for (QDomNode n = rule.firstChild(); !n.isNull(); n = n.nextSibling()) {
		QDomElement rl = n.toElement();
		if ((!rl.isNull()) && (rl.tagName() == "replacelist")) {
            QStringList wildcards1 = ReplaceAttributes(rl.attribute("wildcards")).split(";",
					QString::SkipEmptyParts);
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
    ctx->templatenamews = Utilities::RemoveSpaces(name, false); //TODO underscores or not ?
    ExportCommon::exportInfo("Template: %s from [%s] to [%s]\n", name.toStdString().c_str(),
            path.toStdString().c_str(), dest.toStdString().c_str());

	ExportCommon::copyTemplate(
			QDir::current().relativeFilePath(
                    ctx->outputDir.absoluteFilePath(path)), ctx->outputDir.absoluteFilePath(dest), ctx, isPlugin, include, exclude);
	return true;
}

bool ExportXml::RuleImage(int width, int height, QString dst, ImageTypes type) {
	ExportCommon::exportInfo("Image(Type %d): %dx%d %s\n", type, width, height,
			dst.toStdString().c_str());
    if(type == e_appIcon)
        return ExportCommon::appIcon(ctx, width, height, dst);
    else if(type == e_tvIcon)
        return ExportCommon::tvIcon(ctx, width, height, dst);
    else if(type == e_splashVertical)
        return ExportCommon::splashVImage(ctx, width, height, dst);
    else if(type == e_splashHorizontal)
        return ExportCommon::splashHImage(ctx, width, height, dst);
    return false;
}
