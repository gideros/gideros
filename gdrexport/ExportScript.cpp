/*
 * ExporrtXml.cpp
 *
 *  Created on: 22 janv. 2016
 *      Author: Nico
 */

#include "ExportScript.h"
#include "ExportLua.h"
#include "Utilities.h"
#include "ExportCommon.h"
#include <QStandardPaths>
#include <QFileInfo>
#include <QDesktopServices>
#include <QUrl>

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

ExportScript::ExportScript() {
}

void ExportScript::SetupProperties(ExportContext *ctx)
{
	this->ctx = ctx;
    ctx->basews = Utilities::RemoveSpaces(ctx->base, Utilities::NODIGIT);
	if (ctx->props.empty())
	{
	//Fill properties: System
#ifdef Q_OS_WIN32
		ctx->props["sys.exeExtension"]=".exe";
#else
	ctx->props["sys.exeExtension"] = "";
#endif
	ctx->props["sys.cacheDir"] = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
#ifdef Q_OS_MACX
	ctx->props["sys.giderosDir"] = QDir::currentPath()+"/../..";
#else
	ctx->props["sys.giderosDir"] = QDir::currentPath();
#endif
	ctx->props["sys.toolsDir"] = QDir::currentPath()+"/Tools";
	ctx->props["sys.homeDir"] = QDir::homePath();
	ctx->props["sys.exportDir"] = ctx->exportDir.absolutePath();
    ctx->props["sys.exportType"]=QString(ctx->player?"player":(ctx->assetsOnly?"assets":"full"));
    ctx->props["sys.projectDir"]=QFileInfo(ctx->projectFileName_).path();
    ctx->props["sys.projectFile"]=ctx->projectFileName_;
	//Fill properties: Project
	ctx->props["project.name"] = ctx->base;
	ctx->props["project.namews"] = ctx->basews;
	ctx->props["project.version"] = ctx->properties.version;
	ctx->props["project.platform"] = ctx->platform;
	ctx->props["project.app_name"] = ctx->appName;
	ctx->props["project.version_code"] = QString::number(
			ctx->properties.version_code);
	ctx->props["project.build_number"] = QString::number(
			ctx->properties.build_number);
	ctx->props["project.autorotation"] = QString::number(
			ctx->properties.autorotation);
	ctx->props["project.orientation"] = QString::number(ctx->properties.orientation);
	ctx->props["project.disableSplash"] = QString::number(ctx->properties.disableSplash?1:0);
	ctx->props["project.backgroundColor"] = ctx->properties.backgroundColor;
	ctx->props["project.ios_bundle"] = ctx->properties.ios_bundle;
	ctx->props["project.atv_bundle"] = ctx->properties.atv_bundle;
	ctx->props["project.macos_bundle"] = ctx->properties.macos_bundle;
	ctx->props["project.html5_fbinstant"] = QString::number(ctx->properties.html5_fbinstant?1:0);
	ctx->props["project.html5_pwa"] = QString::number(ctx->properties.html5_pwa?1:0);
	ctx->props["project.html5_pack"] = QString::number(ctx->properties.html5_pack?1:0);
	ctx->props["project.html5_wasm"] = QString::number(ctx->properties.html5_wasm?1:0);

	//Fill in passed arguments
    QHash<QString, QString>::iterator i;
        for (i = ctx->args.begin(); i != ctx->args.end(); ++i)
        	ctx->props["args."+i.key()] = i.value();
	}
}

QMap<QString, QString> ExportScript::availableTargets() {
	QMap < QString, QString > xmlExports;
	QDir sourceDir(TEMPLATES_PATH);
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

QMap<QString, QString> ExportScript::availablePlugins() {
	return ProjectProperties::availablePlugins();
}

bool ExportScript::OpDownload(QString source,QString dest)
{
    return ExportCommon::download(ctx,source,dest);
}

bool ExportScript::OpUnzip(QString source,QString dest)
{
    return ExportCommon::unzip(ctx,source,dest);
}

void ExportScript::OpExportAllFilesTxt()
{
    ExportCommon::exportAllfilesTxt(ctx);
}
void ExportScript::OpExportLuaFilesTxt()
{
    ExportCommon::exportLuafilesTxt(ctx);
}
void ExportScript::OpExportPropertiesBin()
{
    ExportCommon::exportPropertiesBin(ctx);
}
void ExportScript::OpExportAssets(QStringList jets,QStringList noencExt,bool compile)
{
    for (int i=0;i<jets.count();i++)
        ctx->jetset << jets[i];
    for (int i=0;i<noencExt.count();i++)
        ctx->noEncryptionExt.insert(noencExt[i]);
    ExportCommon::exportAssets(ctx, compile);
}
void ExportScript::OpApplyPlugins()
{
    ExportCommon::applyPlugins(ctx);
}
void ExportScript::OpInitPlugins()
{
    ExportCommon::initPlugins(ctx);
}
void ExportScript::OpRequestPlugin(QString name)
{
    ExportCommon::requestPlugin(ctx,name);
}

bool ExportScript::OpLua(QString file,QString content)
{
	if (file.isEmpty())
		return ExportLUA_CallCode(ctx,this,content.toStdString().c_str());
	else
		return ExportLUA_CallFile(ctx,this,file.toStdString().c_str());
}

bool ExportScript::OpExec(QString cmd, QStringList args, QProcessEnvironment env) {
    ExportCommon::exportInfo("Exec: %s [%s] into %s\n", cmd.toStdString().c_str(),args.join(' ').toStdString().c_str(),
			ctx->outputDir.path().toStdString().c_str());
    int err = Utilities::processOutput(cmd, args, ctx->outputDir.path(), env,false);
	ExportCommon::exportInfo("Exec returned: %d\n", err);
	return (err == 0);
}

bool ExportScript::OpMkdir(QString cmd) {
	ExportCommon::exportInfo("MkDir: %s\n", cmd.toStdString().c_str());
	return ctx->outputDir.mkpath(cmd);
}

bool ExportScript::OpRmdir(QString cmd) {
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

bool ExportScript::OpCd(QString cmd) {
	ExportCommon::exportInfo("Cd: %s\n", cmd.toStdString().c_str());
	return ctx->outputDir.cd(cmd);
}

bool ExportScript::OpRm(QString cmd) {
	ExportCommon::exportInfo("Rm: %s\n", cmd.toStdString().c_str());
	ctx->outputDir.remove(cmd);
	return !ctx->outputDir.exists(cmd);
}

bool ExportScript::OpCp(QString src, QString dst) {
	ExportCommon::exportInfo("Cp: %s -> %s\n", src.toStdString().c_str(),
			dst.toStdString().c_str());
	ctx->outputDir.remove(dst);
	return QFile::copy(ctx->outputDir.absoluteFilePath(src),
			ctx->outputDir.absoluteFilePath(dst));
}

bool ExportScript::OpMv(QString src, QString dst) {
	ExportCommon::exportInfo("Mv: %s -> %s\n", src.toStdString().c_str(),
			dst.toStdString().c_str());
	return ctx->outputDir.rename(src, dst);
}

bool ExportScript::OpAsk(QString key,QString title,QString question,QString def,QString uid) {
	char *ret=ExportCommon::askString(title.toUtf8().data(),question.toUtf8().data(),def.toUtf8().data(),IsSecret(key),uid.toUtf8().data());
	QString val=QString::fromUtf8(ret);
	free(ret);
	ExportCommon::exportInfo("Ask: %s -> %s\n", key.toStdString().c_str(),
			SecretVal(key,val).toStdString().c_str());
	SetProperty(key,val);
	return true;
}

bool ExportScript::OpOpenUrl(QString url)
{
	return QDesktopServices::openUrl(QUrl(url));
}

void ExportScript::SetProperty(QString k,QString v)
{
	if (!(ctx->props[k].isEmpty()))
		ctx->props[k]=v;
	else
		lprops[k]=v;
}

QString ExportScript::GetProperty(QString k)
{
	if (!(ctx->props[k].isEmpty()))
		return ctx->props[k];
	return lprops[k];
}

bool ExportScript::OpTemplate(QString name, QString path, QString dest, QStringList include, QStringList exclude, QList<TemplateReplacements> replacementsList, bool addon) {
    for (TemplateReplacements tr: replacementsList) {
        QStringList wildcards1 = tr.wildcards;
        QList < QPair<QByteArray, QByteArray> > replaceList1;
        for (TemplateReplacement r: tr.replacements) {
            QString orig=r.original;
            QString by=r.replacement;
            bool force=r.force;
            if (!force)
                replaceList1
                        << qMakePair(
                                by.toUtf8(),
                                QString("").toUtf8());
            if (r.mode==TemplateReplacement::REPLACE)
                replaceList1
                        << qMakePair(
                                orig.toUtf8(),
                                by.toUtf8());
            else if (r.mode==TemplateReplacement::PREPEND)
                    replaceList1
                            << qMakePair(
                                    orig.toUtf8(),
                                    by.toUtf8()+"\n"+orig.toUtf8());
            else if (r.mode==TemplateReplacement::APPEND)
                    replaceList1
                            << qMakePair(
                                    orig.toUtf8(),
                                    orig.toUtf8()+"\n"+by.toUtf8());
        }

        ctx->wildcards << wildcards1;
        ctx->replaceList << replaceList1;
	}

	ctx->templatename = name;
    ctx->templatenamews = Utilities::RemoveSpaces(name, Utilities::NODIGIT); //TODO underscores or not ?
    ExportCommon::exportInfo("Template: %s from [%s] to [%s]\n", name.toStdString().c_str(),
            path.toStdString().c_str(), dest.toStdString().c_str());

	ExportCommon::copyTemplate(
			QDir::current().relativeFilePath(
                    ctx->outputDir.absoluteFilePath(path)), ctx->outputDir.absoluteFilePath(dest), ctx, addon, include, exclude);
	return true;
}

bool ExportScript::OpImage(int width, int height, QString dst, ImageTypes type, bool alpha) {
	ExportCommon::exportInfo("Image(Type %d): %dx%d %s\n", type, width, height,
			dst.toStdString().c_str());
    if(type == e_appIcon)
        return ExportCommon::appIcon(ctx, width, height, dst,alpha);
    else if(type == e_tvIcon)
        return ExportCommon::tvIcon(ctx, width, height, dst,alpha);
    else if(type == e_splashVertical)
        return ExportCommon::splashVImage(ctx, width, height, dst,alpha);
    else if(type == e_splashHorizontal)
        return ExportCommon::splashHImage(ctx, width, height, dst,alpha);
    return false;
}
