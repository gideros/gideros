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
#include <math.h>
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

void ExportCommon::resizeImage(QImage *image, int width, int height, QString output, int quality){
    int iwidth = image->width(); //image width
    int iheight = image->height(); //image height
    int rwidth = width; //resampled width
    int rheight = height; //resampled height

    float k_w = fabs(1-(float)width/(float)iwidth); //width scaling coef
    float k_h = fabs(1-(float)height/(float)iheight); //height scaling koef
    int dst_x = 0;
    int dst_y = 0;

    if(iwidth < width && iheight >= height){
        rheight = round((iheight * width) / iwidth);
    }
    else if(iwidth >= width && iheight < height){
        rwidth = round((iwidth * height) / iheight);
    }
    else{
        //use smallest
        if(k_h < k_w){
            rwidth = round((iwidth * height) / iheight);
        }
        else{
            rheight = round((iheight * width) / iwidth);
        }
    }

    //new width is bigger than existing
    if(rwidth > width){
            dst_x = (rwidth - width) / 2;
    }

    //new height is bigger than existing
    if(rheight > height){
        dst_y = (rheight - height) / 2;
    }

    image->scaled(rwidth, rheight, Qt::KeepAspectRatio, Qt::SmoothTransformation)
            .copy(dst_x, dst_y, width, height)
            .save(output, "png", quality);
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
    resizeImage(ctx->appicon, width, height, ctx->outputDir.absoluteFilePath(output), 100);
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
            fprintf(stderr, "TV icon %s not found or not readable\n",
                src.toStdString().c_str());
    }
    if (ctx->tvicon->isNull())
        return false;
    exportInfo("Generating TV icon (%dx%d)\n",width,height);
    resizeImage(ctx->tvicon, width, height, ctx->outputDir.absoluteFilePath(output), 100);
    return true;
}

bool ExportCommon::splashHImage(ExportContext *ctx, int width, int height, QString output) {
    if (ctx->splash_h_image == NULL) {
        QDir path(QFileInfo(ctx->projectFileName_).path());
        if (ctx->properties.splash_h_image.isEmpty())
            return true;
        QString appicon = ctx->properties.splash_h_image;
        for (std::size_t i = 0; i < ctx->fileQueue.size(); ++i) {
            const QString& s1 = ctx->fileQueue[i].first;
            const QString& s2 = ctx->fileQueue[i].second;
            if (s1 == appicon) {
                appicon = s2;
                break;
            }
        }
        QString src = path.absoluteFilePath(appicon);
        ctx->splash_h_image = new QImage(src);
        if (ctx->splash_h_image->isNull())
            fprintf(stderr, "Splash horizontal %s not found or not readable\n",
                src.toStdString().c_str());
    }
    if (ctx->splash_h_image->isNull())
        return false;
    exportInfo("Generating splash horizontal (%dx%d)\n",width,height);
    resizeImage(ctx->splash_h_image, width, height, ctx->outputDir.absoluteFilePath(output),-1);
    return true;
}

bool ExportCommon::splashVImage(ExportContext *ctx, int width, int height, QString output) {
    if (ctx->splash_v_image == NULL) {
        QDir path(QFileInfo(ctx->projectFileName_).path());
        if (ctx->properties.splash_v_image.isEmpty())
            return true;
        QString appicon = ctx->properties.splash_v_image;
        for (std::size_t i = 0; i < ctx->fileQueue.size(); ++i) {
            const QString& s1 = ctx->fileQueue[i].first;
            const QString& s2 = ctx->fileQueue[i].second;
            if (s1 == appicon) {
                appicon = s2;
                break;
            }
        }
        QString src = path.absoluteFilePath(appicon);
        ctx->splash_v_image = new QImage(src);
        if (ctx->splash_v_image->isNull())
            fprintf(stderr, "Splash vertical %s not found or not readable\n",
                src.toStdString().c_str());
    }
    if (ctx->splash_v_image->isNull())
        return false;
    exportInfo("Generating splash vertical (%dx%d)\n",width,height);
    resizeImage(ctx->splash_v_image, width, height, ctx->outputDir.absoluteFilePath(output),-1);
    return true;
}

void ExportCommon::exportAssets(ExportContext *ctx, bool compileLua) {
	QStringList allluafiles;
	QStringList allluafiles_abs;

	if (ctx->fileQueue.size()==0) //No assets -> Player
		return;

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

        exportInfo("Exporting %s\n",s1.toUtf8().constData());
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
            QString filename = ctx->allfiles_abs[i];
			QString ext = QFileInfo(filename).suffix().toLower();
            exportInfo("Encrypting %s [%s]\n",filename.toUtf8().constData(),ext.toUtf8().constData());
			if (ext != "lua" && ext != "png" && ext != "jpeg" && ext != "jpg"
					&& ext != "wav")
				continue;

			QByteArray encryptionKey =
					(ext == "lua") ? ctx->codeKey : ctx->assetsKey;

			QFile fis(filename);
			if (!fis.open(QIODevice::ReadOnly))
            {
                exportError("Failed to open %s\n",filename.toUtf8().constData());
				continue;
            }
			QByteArray data = fis.readAll();
			fis.close();

			int ks = encryptionKey.size();
			for (int j = 32; j < data.size(); ++j)
				data[j] = data[j]
						^ encryptionKey[((j * 13) + ((j / ks) * 31)) % ks];

			QFile fos(filename);
			if (!fos.open(QIODevice::WriteOnly))
            {
                exportError("Failed to save %s\n",filename.toUtf8().constData());
                continue;
            }
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
	if (ctx->fileQueue.size()==0) //No assets -> Player
		return;
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
	if (ctx->fileQueue.size()==0) //No assets -> Player
		return;
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
	if (ctx->fileQueue.size()==0) //No assets -> Player
		return;
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
		if ((!xml.isEmpty())&&((*it).enabled))
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

void ExportCommon::exportError(const char *fmt,...)
{
	va_list va;
	va_start(va,fmt);
	vfprintf(stderr,fmt,va);
	va_end(va);
	fflush(stderr);
}

int ExportCommon::progressMax=0;
int ExportCommon::progressCur=0;

void ExportCommon::progressSteps(int steps)
{
	progressMax+=steps;
	exportInfo("$:$%d\n",progressMax);
}

void ExportCommon::progressStep(const char *title)
{
	progressCur++;
	exportInfo("$:$%d:%d:%s\n",progressMax,progressCur,title);
}

char *ExportCommon::askString(const char *title, const char *question, const char *def)
{
	exportInfo("?:?S%s|%s|%s\n",title,question,def);
	char str[512];
	fgets(str, 511, stdin);
	int i = strlen(str)-1;
    if (str[i] == '\n')
    	str[i] = '\0';
    return strdup(str);
}
