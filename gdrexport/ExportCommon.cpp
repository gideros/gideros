/*
 * ExportCommon.cpp
 *
 *  Created on: 22 janv. 2016
 *      Author: Nico
 */

#include "ExportCommon.h"
#include "Utilities.h"
#include <bytebuffer.h>
#include <QProcess>
#include <QImage>

void ExportCommon::copyTemplate(QString templatePath, ExportContext *ctx) {
	QDir dir = QDir::currentPath();
	dir.cd(templatePath);

	ctx->renameList << qMakePair(ctx->templatename, ctx->base);
	ctx->renameList << qMakePair(ctx->templatenamews, ctx->basews);

	if (ctx->assetsOnly)
		Utilities::copyFolder(dir, ctx->outputDir, ctx->renameList,
				ctx->wildcards, ctx->replaceList,
				QStringList() << "libgideros.so" << "libgideros.a"
						<< "gideros.jar" << "gideros.dll" << "libgideros.dylib"
						<< "libgideros.1.dylib" << "gideros.WindowsPhone.lib"
						<< "gideros.Windows.lib" << "WindowsDesktopTemplate.exe"
						<< "MacOSXDesktopTemplate", QStringList());
	else
		Utilities::copyFolder(dir, ctx->outputDir, ctx->renameList,
				ctx->wildcards, ctx->replaceList, QStringList() << "*",
				QStringList());
	ctx->renameList.clear();
	ctx->wildcards.clear();
	ctx->replaceList.clear();
}

bool ExportCommon::appIcon(ExportContext *ctx, int width, int height,
		QString output) {
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
	ctx->appicon->scaled(width, height, Qt::KeepAspectRatio).save(
			ctx->outputDir.absoluteFilePath(output));
	return true;
}

void ExportCommon::exportAssets(ExportContext *ctx, bool compileLua) {
	QStringList allluafiles;
	QStringList allluafiles_abs;
	ctx->allfiles.clear();
	ctx->allfiles_abs.clear();
	ctx->luafiles.clear();
	ctx->luafiles_abs.clear();

	QDir path(QFileInfo(ctx->projectFileName_).path());

	for (std::size_t i = 0; i < ctx->fileQueue.size(); ++i) {
		const QString& s1 = ctx->fileQueue[i].first;
		const QString& s2 = ctx->fileQueue[i].second;

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
		for (int i = 0; i < allluafiles_abs.size(); ++i) {
			QString file = "\"" + allluafiles_abs[i] + "\"";
			QProcess::execute("Tools/luac -o " + file + " " + file);
		}
	}

	// encrypt lua, png, jpg, jpeg and wav files
	if (true) {
		for (int i = 0; i < ctx->allfiles_abs.size(); ++i) {
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
