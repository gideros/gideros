/*
 * ExportCommon.cpp
 *
 *  Created on: 22 janv. 2016
 *      Author: Nico
 */

#include "ExportCommon.h"
#include "Utilities.h"
#include <bytebuffer.h>
#include "ExportXml.h"
#include <stdio.h>
#include <stdarg.h>
#include <QProcess>
#include <QImage>
#include <QCoreApplication>

static QString quote(const QString &str)
{
    return "\"" + str + "\"";
}

void ExportCommon::copyTemplate(QString templatePath, QString templateDest, ExportContext *ctx, bool isPlugin) {
	QDir dir = QDir::currentPath();
	dir.cd(templatePath);

	exportInfo("Processing template\n");
    QDir dir2 = QDir(ctx->outputDir);
    if(!templateDest.isEmpty()){
        dir2 = QDir(templateDest);
    }


    if(!isPlugin){
        ctx->renameList << qMakePair(ctx->templatename, ctx->base);
        ctx->renameList << qMakePair(ctx->templatenamews, ctx->basews);
    }

	if (ctx->assetsOnly)
        Utilities::copyFolder(dir, dir2, ctx->renameList,
				ctx->wildcards, ctx->replaceList,
				QStringList() << "libgideros.so" << "libgideros.a"
						<< "gideros.jar" << "gideros.dll" << "gid.dll" << "libgideros.dylib"
						<< "libgideros.1.dylib" << "gideros.WindowsPhone.lib"
						<< "gideros.Windows.lib" << "WindowsDesktopTemplate.exe"
						<< "MacOSXDesktopTemplate", QStringList());
	else
        Utilities::copyFolder(dir, dir2, ctx->renameList,
				ctx->wildcards, ctx->replaceList, QStringList() << "*",
				QStringList());
	ctx->renameList.clear();
	ctx->wildcards.clear();
	ctx->replaceList.clear();
}

bool ExportCommon::appIcon(ExportContext *ctx, int width, int height, QString output) {
	if (ctx->appicon == NULL) {
		QDir path(QFileInfo(ctx->projectFileName_).path());
		if (ctx->properties.app_icon.isEmpty())
			return true;
		QString appicon = ctx->properties.app_icon;
		for (std::size_t i = 0; i < ctx->fileQueue.size(); ++i) {
			const QString& s1 = ctx->fileQueue[i].first;
			const QString& s2 = ctx->fileQueue[i].second;
			if (s1 == appicon) {
				appicon = s2;
				break;
			}
		}
		QString src = path.absoluteFilePath(appicon);
		ctx->appicon = new QImage(src);
		if (ctx->appicon->isNull())
			fprintf(stderr, "App icon %s not found or not readable\n",
				src.toStdString().c_str());
	}
	if (ctx->appicon->isNull())
		return false;
	exportInfo("Generating app icon (%dx%d)\n",width,height);
    ctx->appicon->scaled(width, height, Qt::KeepAspectRatio, Qt::SmoothTransformation).save(
            ctx->outputDir.absoluteFilePath(output), "png", 100);
	return true;
}

bool ExportCommon::tvIcon(ExportContext *ctx, int width, int height, QString output) {
    if (ctx->tvicon == NULL) {
        QDir path(QFileInfo(ctx->projectFileName_).path());
        if (ctx->properties.tv_icon.isEmpty())
            return true;
        QString tvicon = ctx->properties.tv_icon;
        for (std::size_t i = 0; i < ctx->fileQueue.size(); ++i) {
            const QString& s1 = ctx->fileQueue[i].first;
            const QString& s2 = ctx->fileQueue[i].second;
            if (s1 == tvicon) {
                tvicon = s2;
                break;
            }
        }
        QString src = path.absoluteFilePath(tvicon);
        ctx->tvicon = new QImage(src);
        if (ctx->tvicon->isNull())
            fprintf(stderr, "App icon %s not found or not readable\n",
                src.toStdString().c_str());
    }
    if (ctx->tvicon->isNull())
        return false;
    exportInfo("Generating app icon (%dx%d)\n",width,height);
    ctx->tvicon->scaled(width, height, Qt::KeepAspectRatio, Qt::SmoothTransformation).save(
            ctx->outputDir.absoluteFilePath(output), "png", 100);
    return true;
}

bool ExportCommon::splashHImage(ExportContext *ctx, int width, int height, QString output) {
    if (ctx->splashhimage == NULL) {
        QDir path(QFileInfo(ctx->projectFileName_).path());
        if (ctx->properties.splash_h_image.isEmpty())
            return true;
        QString splashhimage = ctx->properties.splash_h_image;
        for (std::size_t i = 0; i < ctx->fileQueue.size(); ++i) {
            const QString& s1 = ctx->fileQueue[i].first;
            const QString& s2 = ctx->fileQueue[i].second;
            if (s1 == splashhimage) {
                splashhimage = s2;
                break;
            }
        }
        QString src = path.absoluteFilePath(splashhimage);
        ctx->splashhimage = new QImage(src);
        if (ctx->splashhimage->isNull())
            fprintf(stderr, "App icon %s not found or not readable\n",
                src.toStdString().c_str());
    }
    if (ctx->splashhimage->isNull())
        return false;
    exportInfo("Generating app icon (%dx%d)\n",width,height);
    ctx->splashhimage->scaled(width, height, Qt::KeepAspectRatio, Qt::SmoothTransformation).save(
            ctx->outputDir.absoluteFilePath(output), "png", 100);
    return true;
}

bool ExportCommon::splashVImage(ExportContext *ctx, int width, int height, QString output) {
    if (ctx->splashvimage == NULL) {
        QDir path(QFileInfo(ctx->projectFileName_).path());
        if (ctx->properties.splash_v_image.isEmpty())
            return true;
        QString splashvimage = ctx->properties.splash_v_image;
        for (std::size_t i = 0; i < ctx->fileQueue.size(); ++i) {
            const QString& s1 = ctx->fileQueue[i].first;
            const QString& s2 = ctx->fileQueue[i].second;
            if (s1 == splashvimage) {
                splashvimage = s2;
                break;
            }
        }
        QString src = path.absoluteFilePath(splashvimage);
        ctx->splashvimage = new QImage(src);
        if (ctx->splashvimage->isNull())
            fprintf(stderr, "App icon %s not found or not readable\n",
                src.toStdString().c_str());
    }
    if (ctx->splashvimage->isNull())
        return false;
    exportInfo("Generating app icon (%dx%d)\n",width,height);
    ctx->splashvimage->scaled(width, height, Qt::KeepAspectRatio, Qt::SmoothTransformation).save(
            ctx->outputDir.absoluteFilePath(output), "png", 100);
    return true;
}

void ExportCommon::exportAssets(ExportContext *ctx, bool compileLua) {
	QStringList allluafiles;
	QStringList allluafiles_abs;

	exportInfo("Exporting assets\n");
    for (std::size_t i = 0; i < ctx->folderList.size(); ++i)
    	ctx->outputDir.mkdir(ctx->folderList[i]);

	ctx->allfiles.clear();
	ctx->allfiles_abs.clear();
	ctx->luafiles.clear();
	ctx->luafiles_abs.clear();

	QDir path(QFileInfo(ctx->projectFileName_).path());

    ExportCommon::progressSteps(ctx->fileQueue.size());
	for (std::size_t i = 0; i < ctx->fileQueue.size(); ++i) {
		const QString& s1 = ctx->fileQueue[i].first;
		const QString& s2 = ctx->fileQueue[i].second;

        ExportCommon::progressStep(s1.toUtf8().constData());

		QString src = QDir::cleanPath(path.absoluteFilePath(s2));
		QString dst = QDir::cleanPath(ctx->outputDir.absoluteFilePath(s1));

		QString suffix = QFileInfo(dst).suffix().toLower();
		if ((!ctx->jetset.isEmpty()) && (!ctx->jetset.contains(suffix)))
			dst += ".jet";

		ctx->allfiles.push_back(s1);
		ctx->allfiles_abs.push_back(dst);

		if (QFileInfo(src).suffix().toLower() == "lua") {
			allluafiles.push_back(s1);
			allluafiles_abs.push_back(dst);

			if (std::find(ctx->topologicalSort.begin(),
					ctx->topologicalSort.end(), std::make_pair(s2, true))
					== ctx->topologicalSort.end()) {
				ctx->luafiles.push_back(s1);
				ctx->luafiles_abs.push_back(dst);
			}
		}

		QFile::remove(dst);
		QFile::copy(src, dst);
	}

#if 0
	// compile lua files
	if (false)
	{
		compileThread_ = new CompileThread(ctx->luafiles_abs, false, "", QString(), this);
		compileThread_->start();
		compileThread_->wait();
		delete compileThread_;
	}
#endif

	// compile lua files (with luac)
	// disable compile with luac for iOS because 64 bit version
	// http://giderosmobile.com/forum/discussion/5380/ios-8-64bit-only-form-feb-2015
	if (compileLua) {
		exportInfo("Compiling lua\n");
        QDir toolsDir = QDir(QCoreApplication::applicationDirPath());
        #if defined(Q_OS_WIN)
            QString luac = toolsDir.filePath("luac.exe");
        #else
            QString luac = toolsDir.filePath("luac");
        #endif
        ExportCommon::progressSteps(allluafiles_abs.size());
		for (int i = 0; i < allluafiles_abs.size(); ++i) {
	        ExportCommon::progressStep(allluafiles_abs[i].toUtf8().constData());
			QString file = "\"" + allluafiles_abs[i] + "\"";
            QProcess::execute(quote(luac) + " -o " + file + " " + file);
		}
	}

	// encrypt lua, png, jpg, jpeg and wav files
	if (true) {
		exportInfo("Encrypting assets\n");
        ExportCommon::progressSteps(ctx->allfiles_abs.size());
		for (int i = 0; i < ctx->allfiles_abs.size(); ++i) {
	        ExportCommon::progressStep(ctx->allfiles_abs[i].toUtf8().constData());
			QString ext = QFileInfo(ctx->allfiles[i]).suffix().toLower();
			if (ext != "lua" && ext != "png" && ext != "jpeg" && ext != "jpg"
					&& ext != "wav")
				continue;

			QByteArray encryptionKey =
					(ext == "lua") ? ctx->codeKey : ctx->assetsKey;

			QString filename = ctx->allfiles_abs[i];

			QFile fis(filename);
			if (!fis.open(QIODevice::ReadOnly))
				continue;
			QByteArray data = fis.readAll();
			fis.close();

			int ks = encryptionKey.size();
			for (int j = 32; j < data.size(); ++j)
				data[j] = data[j]
						^ encryptionKey[((j * 13) + ((j / ks) * 31)) % ks];

			QFile fos(filename);
			if (!fos.open(QIODevice::WriteOnly))
				continue;
			fos.write(data);
			fos.close();
		}
	}

	// compress lua files
	if (false) {
		for (int i = 0; i < ctx->luafiles_abs.size(); ++i) {
			QString file = "\"" + ctx->luafiles_abs[i] + "\"";
			QProcess::execute(
					"Tools/lua Tools/LuaSrcDiet.lua --quiet " + file + " -o "
							+ file);
		}
	}

	ctx->assetfiles = ctx->allfiles;
	ctx->assetfiles_abs = ctx->allfiles_abs;
}

void ExportCommon::exportAllfilesTxt(ExportContext *ctx) {
	exportInfo("Writing files info\n");
	QFile file(
			QDir::cleanPath(ctx->outputDir.absoluteFilePath("allfiles.txt")));
	if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		QTextStream out(&file);

		for (int i = 0; i < ctx->assetfiles.size(); ++i) {
			QString file = ctx->assetfiles[i];
			QString suffix = QFileInfo(file).suffix().toLower();
			if (!ctx->jetset.contains(suffix))
				file += "*";
			out << file << "\n";
		}
	}
}

void ExportCommon::exportLuafilesTxt(ExportContext *ctx) {
	exportInfo("Writing lua files info\n");
	QString filename = "luafiles.txt";
	if (!ctx->jetset.isEmpty())
		filename += ".jet";
	QFile file(QDir::cleanPath(ctx->outputDir.absoluteFilePath(filename)));
	if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		QTextStream out(&file);

		for (int i = 0; i < ctx->luafiles.size(); ++i)
			out << ctx->luafiles[i] << "\n";
	}
	ctx->allfiles.push_back(filename);
	ctx->allfiles_abs.push_back(
			QDir::cleanPath(ctx->outputDir.absoluteFilePath(filename)));
}

void ExportCommon::exportPropertiesBin(ExportContext *ctx) {
	exportInfo("Writing project properties\n");
	QString filename = "properties.bin";
	if (!ctx->jetset.isEmpty())
		filename += ".jet";
	QFile file(QDir::cleanPath(ctx->outputDir.absoluteFilePath(filename)));
	if (file.open(QIODevice::WriteOnly)) {

		ByteBuffer buffer;

		buffer << ctx->properties.scaleMode;
		buffer << ctx->properties.logicalWidth;
		buffer << ctx->properties.logicalHeight;

		buffer << (int) ctx->properties.imageScales.size();
		for (size_t i = 0; i < ctx->properties.imageScales.size(); ++i) {
			buffer << ctx->properties.imageScales[i].first.toUtf8().constData();
			buffer << (float) ctx->properties.imageScales[i].second;
		}

		buffer << ctx->properties.orientation;
		buffer << ctx->properties.fps;

		buffer << ctx->properties.retinaDisplay;
		buffer << ctx->properties.autorotation;

		buffer << (ctx->properties.mouseToTouch ? 1 : 0);
		buffer << (ctx->properties.touchToMouse ? 1 : 0);
		buffer << ctx->properties.mouseTouchOrder;

		buffer << ctx->properties.windowWidth;
		buffer << ctx->properties.windowHeight;

		file.write(buffer.data(), buffer.size());
	}
	ctx->allfiles.push_back(filename);
	ctx->allfiles_abs.push_back(
			QDir::cleanPath(ctx->outputDir.absoluteFilePath(filename)));
}

bool ExportCommon::applyPlugins(ExportContext *ctx)
{
	exportInfo("Applying plugins\n");
	QMap<QString,QString> allplugins=ExportXml::availablePlugins();
	for (QSet<ProjectProperties::Plugin>::const_iterator it=ctx->properties.plugins.begin();it!=ctx->properties.plugins.end(); it++)
	{
		QString xml=allplugins[(*it).name];
		if (!xml.isEmpty())
			if (!ExportXml::exportXml(xml,true,ctx))
				return false;
	}
	return true;
}

void ExportCommon::exportInfo(const char *fmt,...)
{
	va_list va;
	va_start(va,fmt);
	vfprintf(stdout,fmt,va);
	va_end(va);
	fflush(stdout);
}

int ExportCommon::progressMax=0;
int ExportCommon::progressCur=0;

void ExportCommon::progressSteps(int steps)
{
	progressMax+=steps;
	exportInfo(":%d\n",progressMax);
}

void ExportCommon::progressStep(const char *title)
{
	progressCur++;
	exportInfo(":%d:%d:%s\n",progressMax,progressCur,title);
}

