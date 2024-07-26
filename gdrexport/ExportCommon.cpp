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
#include "ExportLua.h"
#include <math.h>
#include <stdio.h>
#include <stdarg.h>
#include <QProcess>
#include <QImage>
#include <QPainter>
#include <QBuffer>
#include <QCoreApplication>
#include "filedownloader.h"
#include "cendian.h"
#include <zlib.h>

static QString quote(const QString &str) {
	return "\"" + str + "\"";
}

void ExportCommon::copyTemplate(QString templatePath, QString templateDest,
		ExportContext *ctx, bool isPlugin, QStringList include,
		QStringList exclude) {
	QDir dir = QDir::currentPath();
	if (!dir.cd(templatePath)) {
		exportError("Template source not found:%s\n",
				templatePath.toStdString().c_str());
		return;
	}

	exportInfo("Processing template\n");
	QDir dir2 = QDir(ctx->outputDir);
	if (!templateDest.isEmpty()) {
		dir2 = QDir(templateDest);
	}

	if (!isPlugin) {
		ctx->renameList << qMakePair(ctx->templatename, ctx->base);
		ctx->renameList << qMakePair(ctx->templatenamews, ctx->basews);
	}

	if (!include.count()) {
		if (ctx->assetsOnly) {
			include << "libgideros.so" << "libgiderosvr.so" << "libgideros.a" << "gideros.jar"
					<< "gideros.dll" << "gid.dll" << "libgideros.dylib"
					<< "libgideros.1.dylib" << "gideros.WindowsPhone.lib"
					<< "gideros.Windows.lib" << "WindowsDesktopTemplate.exe"
					<< "MacOSXDesktopTemplate";
		} else
			include << "*";
	}

	Utilities::copyFolder(dir, dir2, ctx->renameList, ctx->wildcards,
			ctx->replaceList, include, exclude);
	ctx->renameList.clear();
	ctx->wildcards.clear();
	ctx->replaceList.clear();
}

QByteArray ExportCommon::resizeImageData(QImage *image, int width, int height,
        int quality, bool withAlpha, QColor fill, int mode, bool paletted) {
    Q_UNUSED(quality);
    int iwidth = image->width(); //image width
    int iheight = image->height(); //image height
    int rwidth = width; //resampled width
    int rheight = height; //resampled height

    float k_w = fabs((float) width / (float) iwidth); //width scaling coef
    float k_h = fabs((float) height / (float) iheight); //height scaling coef
    int dst_x = 0;
    int dst_y = 0;
    bool redraw = false;
    int sx = 0;
    int sy = 0;
    int sw = -1;
    int sh = -1;

    if (mode == 0) {  //show all
        //use smallest
        if (k_h < k_w) {
            rwidth = round((iwidth * height) / iheight);
        } else {
            rheight = round((iheight * width) / iwidth);
        }

        //new width is bigger than existing
        if (width > rwidth) {
            dst_x = (width - rwidth) / 2;
            redraw = true;
        }

        //new height is bigger than existing
        if (height > rheight) {
            dst_y = (height - rheight) / 2;
            redraw = true;
        }
    } else if (mode == 1) {  //crop
        if (k_h < k_w) {
            rheight = round((iheight * width) / iwidth);
        } else {
            rwidth = round((iwidth * height) / iheight);
        }

        if (width < rwidth) {
            sx = (rwidth - width) / 2;
            sw = width;
            redraw = true;
        }

        if (height < rheight) {
            sy = (rheight - height) / 2;
            sh = height;
            redraw = true;
        }
    }

    QImage xform = image->scaled(rwidth, rheight, Qt::KeepAspectRatio,
            Qt::SmoothTransformation);
    if (redraw)  //(dst_x || dst_y)
    {
        QImage larger(width, height, QImage::Format_ARGB32);
        larger.fill(fill);
        QPainter painter(&larger);
        painter.drawImage(dst_x, dst_y, xform, sx, sy, sw, sh);
        painter.end();
        xform = larger;
    }
    if (!withAlpha)
        xform = xform.convertToFormat(QImage::Format_RGB888);
    if (paletted)
        xform = xform.convertToFormat(QImage::Format_Indexed8);
    QBuffer output;
    output.open(QFile::OpenModeFlag::WriteOnly);
    xform.save(&output, "png", 0); //Use maximumt compression for PNG, not quality since png is lossless anyhow
    output.close();
    return output.buffer();
}

void ExportCommon::resizeImage(QImage *image, int width, int height,
        QString output, int quality, bool withAlpha, QColor fill, int mode, bool paletted) {
    Q_UNUSED(quality);
    int iwidth = image->width(); //image width
	int iheight = image->height(); //image height
	int rwidth = width; //resampled width
	int rheight = height; //resampled height

	float k_w = fabs((float) width / (float) iwidth); //width scaling coef
	float k_h = fabs((float) height / (float) iheight); //height scaling coef
	int dst_x = 0;
	int dst_y = 0;
	bool redraw = false;
	int sx = 0;
	int sy = 0;
	int sw = -1;
	int sh = -1;

	if (mode == 0) {  //show all
		//use smallest
		if (k_h < k_w) {
			rwidth = round((iwidth * height) / iheight);
		} else {
			rheight = round((iheight * width) / iwidth);
		}

		//new width is bigger than existing
		if (width > rwidth) {
			dst_x = (width - rwidth) / 2;
			redraw = true;
		}

		//new height is bigger than existing
		if (height > rheight) {
			dst_y = (height - rheight) / 2;
			redraw = true;
		}
	} else if (mode == 1) {  //crop
		if (k_h < k_w) {
			rheight = round((iheight * width) / iwidth);
		} else {
			rwidth = round((iwidth * height) / iheight);
		}

		if (width < rwidth) {
			sx = (rwidth - width) / 2;
			sw = width;
			redraw = true;
		}

		if (height < rheight) {
			sy = (rheight - height) / 2;
			sh = height;
			redraw = true;
		}
	}

	QImage xform = image->scaled(rwidth, rheight, Qt::KeepAspectRatio,
			Qt::SmoothTransformation);
	if (redraw)  //(dst_x || dst_y)
	{
		QImage larger(width, height, QImage::Format_ARGB32);
		larger.fill(fill);
		QPainter painter(&larger);
		painter.drawImage(dst_x, dst_y, xform, sx, sy, sw, sh);
		painter.end();
		xform = larger;
	}
    if (!withAlpha)
        xform = xform.convertToFormat(QImage::Format_RGB888);
    if (paletted)
        xform = xform.convertToFormat(QImage::Format_Indexed8);
    xform.save(output, "png", 0); //Use maximumt compression for PNG, not quality since png is lossless anyhow
}

bool ExportCommon::appIcon(ExportContext *ctx, int width, int height,
        QString output, bool withAlpha, bool paletted) {
	if (ctx->appicon == NULL) {
		QDir path(QFileInfo(ctx->projectFileName_).path());
		if (ctx->properties.app_icon.isEmpty())
			ctx->appicon = new QImage("Tools/gideros-mobile-icon.png");
		else {
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
	}
    if (ctx->appicon->isNull())
        return false;
    exportInfo("Generating app icon (%dx%d)\n", width, height);
    resizeImage(ctx->appicon, width, height,
                ctx->outputDir.absoluteFilePath(output), 100, withAlpha, paletted);
    return true;
}

QByteArray ExportCommon::appIconData(ExportContext *ctx, int width, int height,
        bool withAlpha, bool paletted) {
    if (ctx->appicon == NULL) {
        QDir path(QFileInfo(ctx->projectFileName_).path());
        if (ctx->properties.app_icon.isEmpty())
            ctx->appicon = new QImage("Tools/gideros-mobile-icon.png");
        else {
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
    }
    if (ctx->appicon->isNull())
        return QByteArray();
    exportInfo("Generating app icon (%dx%d)\n", width, height);
    return resizeImageData(ctx->appicon, width, height, 100, withAlpha, paletted);
}

bool ExportCommon::tvIcon(ExportContext *ctx, int width, int height,
		QString output, bool withAlpha) {
	if (ctx->tvicon == NULL) {
		QDir path(QFileInfo(ctx->projectFileName_).path());
		if (ctx->properties.tv_icon.isEmpty())
			ctx->tvicon = new QImage("Tools/gideros-mobile-icon.png");
		else {
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
	}
	if (ctx->tvicon->isNull())
		return false;
	exportInfo("Generating TV icon (%dx%d)\n", width, height);
	resizeImage(ctx->tvicon, width, height,
			ctx->outputDir.absoluteFilePath(output), 100, withAlpha);
	return true;
}

bool ExportCommon::splashHImage(ExportContext *ctx, int width, int height,
		QString output, bool withAlpha) {
	if (ctx->splash_h_image == NULL) {
		QDir path(QFileInfo(ctx->projectFileName_).path());
		if (ctx->properties.splash_h_image.isEmpty())
			ctx->splash_h_image = new QImage("Tools/gideros-mobile-splash.png");
		else {
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
				fprintf(stderr,
						"Splash horizontal %s not found or not readable\n",
						src.toStdString().c_str());
		}
	}
	if (ctx->splash_h_image->isNull())
		return false;

	int mode = ctx->properties.splashScaleMode;
	exportInfo("Generating splash horizontal (%dx%d) scale mode (%d)\n", width,
			height, mode);

	resizeImage(ctx->splash_h_image, width, height,
			ctx->outputDir.absoluteFilePath(output), -1, withAlpha,
			QColor(ctx->properties.backgroundColor), mode);
	return true;
}

bool ExportCommon::splashVImage(ExportContext *ctx, int width, int height,
		QString output, bool withAlpha) {
	if (ctx->splash_v_image == NULL) {
		QDir path(QFileInfo(ctx->projectFileName_).path());
		if (ctx->properties.splash_v_image.isEmpty())
			ctx->splash_v_image = new QImage("Tools/gideros-mobile-splash.png");
		else
		{
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
				fprintf(stderr,
						"Splash vertical %s not found or not readable\n",
						src.toStdString().c_str());
		}
	}
	if (ctx->splash_v_image->isNull())
		return false;
	int mode = ctx->properties.splashScaleMode;
	exportInfo("Generating splash vertical (%dx%d) scale mode (%d)\n", width,
			height, mode);
	resizeImage(ctx->splash_v_image, width, height,
			ctx->outputDir.absoluteFilePath(output), -1, withAlpha,
			QColor(ctx->properties.backgroundColor), mode);
	return true;
}

void ExportCommon::exportAssets(ExportContext *ctx, bool compileLua) {
	if ((ctx->fileQueue.size() == 0) || (ctx->player)) //No assets -> Player
		return;

	ctx->noPackageAbs.clear();

	exportInfo("Exporting assets\n");
	for (std::size_t i = 0; i < ctx->folderList.size(); ++i)
		ctx->outputDir.mkdir(ctx->folderList[i]);

	ctx->allfiles.clear();
	ctx->allfiles_abs.clear();
	ctx->luafiles.clear();
	ctx->luafiles_abs.clear();

	QStringList luafiles_src;

	QDir path(QFileInfo(ctx->projectFileName_).path());

	ExportCommon::progressSteps(ctx->fileQueue.size());
	for (std::size_t i = 0; i < ctx->fileQueue.size(); ++i) {
		const QString& s1 = ctx->fileQueue[i].first;
		const QString& s2 = ctx->fileQueue[i].second;

		exportInfo("Exporting %s\n", s1.toUtf8().constData());
		ExportCommon::progressStep(s1.toUtf8().constData());

		QString src = QDir::cleanPath(path.absoluteFilePath(s2));
		QString dst = QDir::cleanPath(ctx->outputDir.absoluteFilePath(s1));
		QString rdst = QDir::cleanPath(ctx->outputDir.relativeFilePath(s1));

		QString suffix = QFileInfo(dst).suffix().toLower();
		bool isJet = false;
		if ((!ctx->jetset.isEmpty()) && (!ctx->jetset.contains(suffix))) {
			dst += ".jet";
			isJet = true;
		}

		QFile::remove(dst);
		bool copied = false;

		if (QFileInfo(src).suffix().toLower() == "lua") {
			//These files are marked as exclude from execution because they come from plugins
			//But we want these specific files to be ran before other code files, even before init.lua
			bool preload=s1.startsWith("_LuaPlugins_/")&&s1.endsWith("/_preload.lua");
			bool allowExec=(!ctx->properties.mainluaOnly)||(s1=="main.lua");

			if (((std::find(ctx->topologicalSort.begin(),
					ctx->topologicalSort.end(), std::make_pair(s2, true))
					== ctx->topologicalSort.end())&&allowExec)||preload)
			{
				if (preload) {
					ctx->luafiles.push_front(s1);
					ctx->luafiles_abs.push_front(dst);
				}
				else {
					ctx->luafiles.push_back(s1);
					ctx->luafiles_abs.push_back(dst);
				}
				luafiles_src.push_back(src);
				if (!compileLua) {
					ctx->allfiles.push_back(s1);
					ctx->allfiles_abs.push_back(dst);
				} else
					copied = true;
			} else // compile independant lua files (with luac)
			{
				ctx->allfiles.push_back(s1);
				ctx->allfiles_abs.push_back(dst);
				if (compileLua) {
					QDir toolsDir = QDir(
							QCoreApplication::applicationDirPath());
					QDir old = QDir::current();
					QDir::setCurrent(ctx->outputDir.path());
					QString dfile = "\"" + dst + "\"";
					QString sfile = "\"" + rdst + "\"";
					QFile::copy(src, rdst);
#ifdef USE_LUAU_ENGINE
#if defined(Q_OS_WIN)
					QString luac = toolsDir.filePath("luauc.exe");
#else
					QString luac = toolsDir.filePath("luauc");
#endif
					QProcess procWrite;
					procWrite.setStandardOutputFile(dfile);
					procWrite.start(luac, QStringList() << "--compile=binary" << sfile);
					procWrite.waitForFinished();
#else
#if defined(Q_OS_WIN)
					QString luac = toolsDir.filePath("luac.exe");
#else
					QString luac = toolsDir.filePath("luac");
#endif
					QProcess::execute(
							quote(luac) + " -o " + dfile + " " + sfile);
#endif
					if (isJet)
						QFile::remove(rdst);
					QDir::setCurrent(old.path());
				} else {
				}
			}
		} else {
			ctx->allfiles.push_back(s1);
			ctx->allfiles_abs.push_back(dst);
		}

		if (!copied)
			QFile::copy(src, dst);
	}

	// compile building lua files
	if (compileLua) {
		QDir toolsDir = QDir(QCoreApplication::applicationDirPath());
		QDir old = QDir::current();
		QDir::setCurrent(ctx->outputDir.path());

		QString dfile = "";
		QString difile = "";
		QString sfile = "";
		QStringList sfilelst;
		for (int i = 0; i < ctx->luafiles_abs.size(); ++i) {
			dfile = ctx->luafiles_abs[i];
			difile = ctx->luafiles[i];
			QString rdst = QDir::cleanPath(
					ctx->outputDir.relativeFilePath(difile));
			sfile = sfile + " \"" + rdst + "\"";
			sfilelst << rdst;
			QFile::copy(luafiles_src[i], rdst);
		}
		QFileInfo di(dfile);

#ifdef USE_LUAU_ENGINE
#if defined(Q_OS_WIN)
					QString luac = toolsDir.filePath("luauc.exe");
#else
					QString luac = toolsDir.filePath("luauc");
#endif
					QProcess procWrite;
                    procWrite.setStandardOutputFile(dfile+".tmp");
					procWrite.start(luac, QStringList() << "--compile=binary" << sfilelst);
					procWrite.waitForFinished();
#else
#if defined(Q_OS_WIN)
					QString luac = toolsDir.filePath("luac.exe");
#else
					QString luac = toolsDir.filePath("luac");
#endif
                    QProcess::execute(quote(luac) + " -o \"" + dfile + ".tmp\" " + sfile);
#endif

        for (int i = 0; i < ctx->luafiles_abs.size(); ++i) {
			QString rdst = QDir::cleanPath(
					ctx->outputDir.relativeFilePath(ctx->luafiles[i]));
            QFile::remove(rdst);
		}

        QFile::rename(di.absoluteFilePath()+".tmp",di.absoluteFilePath());
		ctx->luafiles.clear();
		ctx->luafiles_abs.clear();
		ctx->luafiles.push_back(difile);
		ctx->luafiles_abs.push_back(dfile);
		ctx->allfiles.push_back(difile);
		ctx->allfiles_abs.push_back(dfile);
		QDir::setCurrent(old.path());
	}

	// encrypt lua, png, jpg, jpeg and wav files
	if (true) {
		exportInfo("Encrypting assets\n");
		ExportCommon::progressSteps(ctx->allfiles_abs.size());
		for (int i = 0; i < ctx->allfiles_abs.size(); ++i) {
			ExportCommon::progressStep(
					ctx->allfiles_abs[i].toUtf8().constData());
			if (ctx->noPackage.count(ctx->allfiles[i]))
				ctx->noPackageAbs.insert(ctx->allfiles_abs[i]);
			if (ctx->noEncryption.count(ctx->allfiles[i]))
				continue; //File marked as non encryotable
			QString filename = ctx->allfiles_abs[i];
			QString ext = QFileInfo(ctx->allfiles[i]).suffix().toLower();
			if (ctx->noEncryptionExt.count(ext))
				continue; //Extension marked as non encryptable
			bool encrypt =
					(ext == "lua") ? ctx->encryptCode : ctx->encryptAssets;
			if (!encrypt)
				continue;
			exportInfo("Encrypting %s [%s]\n", filename.toUtf8().constData(),
					ext.toUtf8().constData());
			/*if (ext != "lua" && ext != "png" && ext != "jpeg" && ext != "jpg"
			 && ext != "wav")
			 continue;*/

			QByteArray encryptionKey =
					(ext == "lua") ? ctx->codeKey : ctx->assetsKey;

			QFile fis(filename);
			if (!fis.open(QIODevice::ReadOnly)) {
				exportError("Failed to open %s\n",
						filename.toUtf8().constData());
				ctx->exportError = true;
				continue;
			}
			QByteArray data = fis.readAll();
			fis.close();

			int ks = encryptionKey.size();
			for (int j = 32; j < data.size(); ++j)
				data[j] = data[j]
						^ encryptionKey[((j * 13) + ((j / ks) * 31)) % ks];

			//Add encryption marker
			unsigned char sig[4] = { 'G', 'x', 0xE7, 0 };
			unsigned long dlength = data.size();
			sig[3] = (ext == "lua") ? 1 : 2;
			sig[0] ^= (dlength >> 24) & 0xFF;
			sig[1] ^= (dlength >> 16) & 0xFF;
			sig[2] ^= (dlength >> 8) & 0xFF;
			sig[3] ^= (dlength >> 0) & 0xFF;
			data.append((char *) sig, 4);

			QFile fos(filename);
			if (!fos.open(QIODevice::WriteOnly)) {
				exportError("Failed to save %s\n",
						filename.toUtf8().constData());
				ctx->exportError = true;
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
	if ((ctx->fileQueue.size() == 0) || (ctx->player)) //No assets -> Player
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
	if ((ctx->fileQueue.size() == 0) || (ctx->player)) //No assets -> Player
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
	if ((ctx->fileQueue.size() == 0) || (ctx->player)) //No assets -> Player
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

        buffer << ctx->appName.toStdString();
	    buffer << ctx->properties.version.toStdString();
	    buffer << ctx->properties.version_code;
	    buffer << ctx->properties.build_number;
        buffer << (unsigned int) QDateTime::currentSecsSinceEpoch();

		file.write(buffer.data(), buffer.size());
	}
	ctx->allfiles.push_back(filename);
	ctx->allfiles_abs.push_back(
			QDir::cleanPath(ctx->outputDir.absoluteFilePath(filename)));
}

bool ExportCommon::initPlugins(ExportContext *ctx) {
	exportInfo("Querying plugins\n");
    QMap < QString, QString > allplugins = ExportScript::availablePlugins();
	for (QSet<ProjectProperties::Plugin>::const_iterator it =
			ctx->properties.plugins.begin();
			it != ctx->properties.plugins.end(); it++) {
		QString xml = allplugins[(*it).name];
		bool en=((*it).enabled);
		if ((!xml.isEmpty()) && en)
		{
			if (!ExportXml::runinitXml(xml, true, ctx))
				return false;
		}
	}
	//Add JSON automatically if not already done for FBInstant
    if ((ctx->deviceFamily == e_Html5)&&ctx->properties.html5_fbinstant) {
    	requestPlugin(ctx,"JSON");
    }
	return true;
}

void ExportCommon::requestPlugin(ExportContext *ctx,QString name) {
	ProjectProperties::Plugin p;
	p.name=name;
	p.enabled=true;
	for (QSet<ProjectProperties::Plugin>::iterator it =
			ctx->properties.plugins.begin();
			it != ctx->properties.plugins.end(); it++) {
		if (it->name==name) { p.properties=it->properties; it=ctx->properties.plugins.erase(it); };
	}

	ctx->properties.plugins.insert(p);
}

bool ExportCommon::applyPlugins(ExportContext *ctx) {
	if (ctx->assetsOnly) //Don't export plugins on asset only
	{
		ExportLUA_DonePlugins(ctx);
		return true;
	}
	exportInfo("Applying plugins\n");
    QMap < QString, QString > allplugins = ExportScript::availablePlugins();
	for (QSet<ProjectProperties::Plugin>::const_iterator it =
			ctx->properties.plugins.begin();
			it != ctx->properties.plugins.end(); it++) {
		QString xml = allplugins[(*it).name];
		bool en=((*it).enabled);
		if ((!xml.isEmpty()) && en)
		{
			if (!ExportXml::exportXml(xml, true, ctx))
				return false;
		}
	}
	ExportLUA_DonePlugins(ctx);
	return true;
}

bool ExportCommon::download(ExportContext *ctx, QString url, QString to) {
	QString filePath = QDir::cleanPath(ctx->outputDir.absoluteFilePath(to));
	QFileInfo fi = QFileInfo(filePath);
	ctx->outputDir.mkpath(fi.dir().absolutePath());
	QFile file(filePath);
	exportInfo("Checking %s\n", url.toStdString().c_str());
	QUrl imageUrl(url);

	quint64 size = 0;
	{
		FileDownloader *m_pImgCtrl = new FileDownloader(imageUrl, true);

		QEventLoop loop;
		loop.connect(m_pImgCtrl, SIGNAL(downloaded()), &loop, SLOT(quit()));
		loop.exec();

		size = m_pImgCtrl->fileSize();

		delete m_pImgCtrl;
	}

	if (file.exists()) {
		if (size == 0) {
			exportInfo("Couldn't check file size for %s, assuming valid\n",
					url.toStdString().c_str());
			return true; //Same file as far as we can tell
		}

		if (size == file.size()) {
			exportInfo("File %s already in cache\n", url.toStdString().c_str());
			return true; //Same file as far as we can tell
		}
	} else {
		if (size == 0) {
			exportError("Failed to determine file size for %s, aborting\n",
					url.toStdString().c_str());
			return false;
		}
	}

	if (file.open(QIODevice::WriteOnly)) {
		exportInfo("Downloading %s (%lld bytes)\n", url.toStdString().c_str(),
				size);

		FileDownloader *m_pImgCtrl = new FileDownloader(imageUrl, false, size);

		QEventLoop loop;
		loop.connect(m_pImgCtrl, SIGNAL(downloaded()), &loop, SLOT(quit()));
		loop.exec();

		QByteArray data = m_pImgCtrl->downloadedData();

		delete m_pImgCtrl;

		if (data.length() > 0) {
			file.write(data);
			return true;
		} else
			exportError("Failed to download %s\n", url.toStdString().c_str());
	} else
		exportError("Can't open file %s\n", to.toStdString().c_str());
	return false;
}

#define GZIP_CHUNK_SIZE 32 * 1024

static bool gzInflate(QByteArray input, QByteArray &output) {
	// Prepare output
	output.clear();

	// Is there something to do?
	if (input.length() > 0) {
		// Prepare inflater status
		z_stream strm;
		strm.zalloc = Z_NULL;
		strm.zfree = Z_NULL;
		strm.opaque = Z_NULL;
		strm.avail_in = 0;
		strm.next_in = Z_NULL;

		// Initialize inflater
		int ret = inflateInit2(&strm, -15);

		if (ret != Z_OK)
			return (false);

		// Extract pointer to input data
		char *input_data = input.data();
		int input_data_left = input.length();

		// Decompress data until available
		do {
			// Determine current chunk size
			int chunk_size = qMin(GZIP_CHUNK_SIZE, input_data_left);

			// Check for termination
			if (chunk_size <= 0)
				break;

			// Set inflater references
			strm.next_in = (unsigned char*) input_data;
			strm.avail_in = chunk_size;

			// Update interval variables
			input_data += chunk_size;
			input_data_left -= chunk_size;

			// Inflate chunk and cumulate output
			do {

				// Declare vars
				char out[GZIP_CHUNK_SIZE];

				// Set inflater references
				strm.next_out = (unsigned char*) out;
				strm.avail_out = GZIP_CHUNK_SIZE;

				// Try to inflate chunk
				ret = inflate(&strm, Z_NO_FLUSH);

				switch (ret) {
				case Z_NEED_DICT:
					ret = Z_DATA_ERROR;
				case Z_DATA_ERROR:
				case Z_MEM_ERROR:
				case Z_STREAM_ERROR:
					// Clean-up
					inflateEnd(&strm);

					// Return
					return (false);
				}

				// Determine decompressed size
				int have = (GZIP_CHUNK_SIZE - strm.avail_out);

				// Cumulate result
				if (have > 0)
					output.append((char*) out, have);

			} while (strm.avail_out == 0);

		} while (ret != Z_STREAM_END);

		// Clean-up
		inflateEnd(&strm);

		// Return
		return (ret == Z_STREAM_END);
	} else
		return (true);
}

#define PACKED __attribute__((packed))
bool ExportCommon::unzip(ExportContext *ctx, QString file, QString dest) {
	QDir toPath(ctx->outputDir);
	toPath.mkpath(dest);
	toPath.cd(dest);
	exportInfo("Unzip %s to %s\n", file.toStdString().c_str(),
			toPath.absolutePath().toStdString().c_str());
	QFile zfile(file);
	if (!zfile.open(QIODevice::ReadOnly)) {
		exportError("Can't open file %s\n", file.toStdString().c_str());
		return false;
	}
	//Attempt to locate central directory: expect EOCD in last 512 bytes of the file
	qint64 fsz = zfile.size();
	if (fsz > 512)
		zfile.seek(fsz - 512);
	QByteArray lastBytes = zfile.read((fsz > 512) ? 512 : fsz);
	const char eocdMarker[5] = { 'P', 'K', 5, 6, 0 };
	int eocd = lastBytes.lastIndexOf(eocdMarker);

	if (eocd > 0) //If central dir was found, just get it
			{
		quint32 cdoff;
		memcpy(&cdoff, lastBytes.constData() + eocd + 16, 4);
		zfile.seek(_letohl(cdoff));
		//Scan central directory
		while (true) {
#pragma pack(push,1)
			struct _ZipHdr {
				quint32 Signature; //	local file header signature     4 bytes  (0x04034b50)
#define ZIPCHDR_SIG 0x02014b50
				quint16 VMade;	//	version made       2 bytes
				quint16 Version;	//	version needed to extract       2 bytes
				quint16 Flags;	//	general purpose bit flag        2 bytes
				quint16 Compression;//	compression method              2 bytes
				quint16 ModTime;	//	last mod file time              2 bytes
				quint16 ModDate;	//	last mod file date              2 bytes
				quint32 Crc32;	//	crc-32                          4 bytes
				quint32 CompSize;	//	compressed size                 4 bytes
				quint32 OrigSize;	//	uncompressed size               4 bytes
				quint16 NameLen;	//  file name length                2 bytes
				quint16 ExtraLen;	//  extra field length              2 bytes
				quint16 CommLen;	//  comm field length              2 bytes
				quint16 DiskNum;	//  disk # start             2 bytes
				quint16 IntAttr;	//  internal attr              2 bytes
				quint32 ExtAttr;	//  external attr              2 bytes
				quint32 Offset;	//  offset of local header              2 bytes
			}PACKED Hdr;
			struct _ZipHdr2 {
				quint32 Signature; //	local file header signature     4 bytes  (0x04034b50)
#define ZIPFHDR_SIG 0x04034b50
				quint16 Version;	//	version needed to extract       2 bytes
				quint16 Flags;	//	general purpose bit flag        2 bytes
				quint16 Compression;//	compression method              2 bytes
				quint16 ModTime;	//	last mod file time              2 bytes
				quint16 ModDate;	//	last mod file date              2 bytes
				quint32 Crc32;	//	crc-32                          4 bytes
				quint32 CompSize;	//	compressed size                 4 bytes
				quint32 OrigSize;	//	uncompressed size               4 bytes
				quint16 NameLen;	//  file name length                2 bytes
				quint16 ExtraLen;	//  extra field length              2 bytes
			}PACKED FHdr;
#pragma pack(pop)
			if (zfile.read((char *) &Hdr, sizeof(Hdr)) != sizeof(Hdr))
				break;
			if (_letohl(Hdr.Signature) != ZIPCHDR_SIG) {
				zfile.seek(zfile.pos() - sizeof(Hdr));
				break;
			}
			if (_letohs(Hdr.Version) > 20) {
				exportError("Unsupported ZIP version for %s [%d]\n",
						file.toStdString().c_str(), _letohs(Hdr.Version));
				return false;
			}
			if ((Hdr.Flags & (~8)) != 0) {
				exportError("Unsupported flags for %s [%04x]\n",
						file.toStdString().c_str(), Hdr.Flags);
				return false;
			}
			if ((Hdr.Compression > 0) && (_letohs(Hdr.Compression) != 8)) {
				exportError("Unsupported compression method for %s [%d]\n",
						file.toStdString().c_str(), _letohs(Hdr.Compression));
				return false;
			}

			QByteArray fname = zfile.read(_letohs(Hdr.NameLen));
			QString lname = QString(fname);
			zfile.read(_letohs(Hdr.ExtraLen));
			zfile.read(_letohs(Hdr.CommLen));

			exportInfo("Extracting %s\n", lname.toStdString().c_str());
			//Grab file data
			qint64 fpos = zfile.pos();
			zfile.seek(_letohl(Hdr.Offset));
			if (zfile.read((char *) &FHdr, sizeof(FHdr)) != sizeof(FHdr))
				break;
			zfile.read(_letohs(FHdr.NameLen));
			zfile.read(_letohs(FHdr.ExtraLen));
			QByteArray fcont = zfile.read(
					Hdr.Compression ?
							_letohl(Hdr.CompSize) : _letohl(Hdr.OrigSize));

			zfile.seek(fpos);

			if (Hdr.Compression) {
				QByteArray decomp;
				if (!gzInflate(fcont, decomp)) {
					exportError("Failed to uncompress %s\n",
							lname.toStdString().c_str());
					break;
				}
				fcont = decomp;
			}
			int unixattr = -1;
			if ((_letohs(Hdr.VMade) >> 8) == 3) //Unix
					{
				unixattr = _letohl(Hdr.ExtAttr) >> 16;
			}

			if ((unixattr >= 0) && ((unixattr & 0120000) == 0120000)) //SymLink
					{
				exportInfo("Link %s\n", lname.toStdString().c_str());
				QFile ofile(toPath.absoluteFilePath(lname));
				ofile.remove();
				QString tgt = QString(fcont);
				if (!QFile::link(tgt, toPath.absoluteFilePath(lname)))
					exportError("Can't make link %s to %s\n",
							lname.toStdString().c_str(),
							tgt.toStdString().c_str());
			} else if (lname.endsWith("/"))
				toPath.mkpath(lname);
			else {
				QFile ofile(toPath.absoluteFilePath(lname));
				if (ofile.open(QIODevice::WriteOnly)) {
					ofile.write(fcont);
					ofile.close();
				} else { //Try to create dir
                    QFileInfo fi(toPath.absoluteFilePath(lname));
                    fi.absoluteDir().mkpath(".");
                    if (ofile.open(QIODevice::WriteOnly)) {
                        ofile.write(fcont);
                        ofile.close();
                    } else { //No joy
                        exportError("Can't open file %s\n",
                                lname.toStdString().c_str());
                        break;
                    }
				}
			}
			if ((unixattr >= 0) && (unixattr & 1)) {
				QFile ofile(toPath.absoluteFilePath(lname));
				ofile.setPermissions(
						QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner
								| QFile::ReadGroup | QFile::ExeGroup
								| QFile::ReadOther | QFile::ExeOther);
			}
		}
	} else
		//Otherwise scan through the files
		while (true) {
#pragma pack(push,1)
			struct _ZipHdr {
				quint32 Signature; //	local file header signature     4 bytes  (0x04034b50)
#define ZIPHDR_SIG 0x04034b50
				quint16 Version;	//	version needed to extract       2 bytes
				quint16 Flags;	//	general purpose bit flag        2 bytes
				quint16 Compression;//	compression method              2 bytes
				quint16 ModTime;	//	last mod file time              2 bytes
				quint16 ModDate;	//	last mod file date              2 bytes
				quint32 Crc32;	//	crc-32                          4 bytes
				quint32 CompSize;	//	compressed size                 4 bytes
				quint32 OrigSize;	//	uncompressed size               4 bytes
				quint16 NameLen;	//  file name length                2 bytes
				quint16 ExtraLen;	//  extra field length              2 bytes
			}PACKED Hdr;
#pragma pack(pop)
			if (zfile.read((char *) &Hdr, sizeof(Hdr)) != sizeof(Hdr))
				break;
			if (_letohl(Hdr.Signature) != ZIPHDR_SIG) {
				zfile.seek(zfile.pos() - sizeof(Hdr));
				break;
			}
			if (_letohs(Hdr.Version) > 20) {
				exportError("Unsupported ZIP version for %s [%d]\n",
						file.toStdString().c_str(), _letohs(Hdr.Version));
				return false;
			}
			if (Hdr.Flags != 0) {
				exportError("Unsupported flags for %s [%04x]\n",
						file.toStdString().c_str(), Hdr.Flags);
				return false;
			}
			if ((Hdr.Compression > 0) && (_letohs(Hdr.Compression) != 8)) {
				exportError("Unsupported compression method for %s [%d]\n",
						file.toStdString().c_str(), _letohs(Hdr.Compression));
				return false;
			}
			QByteArray fname = zfile.read(_letohs(Hdr.NameLen));
			QString lname = QString(fname);
			zfile.read(_letohs(Hdr.ExtraLen));
			exportInfo("Extracting %s\n", lname.toStdString().c_str());
			QByteArray fcont = zfile.read(
					Hdr.Compression ?
							_letohl(Hdr.CompSize) : _letohl(Hdr.OrigSize));
			if (Hdr.Compression) {
				QByteArray decomp;
				if (!gzInflate(fcont, decomp)) {
					exportError("Failed to uncompress %s\n",
							lname.toStdString().c_str());
					break;
				}
				fcont = decomp;
			}
			if (lname.endsWith("/"))
				toPath.mkpath(lname);
			else {
				QFile ofile(toPath.absoluteFilePath(lname));
				if (ofile.open(QIODevice::WriteOnly)) {
					ofile.write(fcont);
					ofile.close();
				} else {
					exportError("Can't open file %s\n",
							lname.toStdString().c_str());
					break;
				}
			}
		}
	zfile.close();
	return true;
}

void ExportCommon::exportInfo(const char *fmt, ...) {
	va_list va;
	va_start(va, fmt);
	vfprintf(stdout, fmt, va);
	va_end(va);
	fflush(stdout);
}

void ExportCommon::exportError(const char *fmt, ...) {
	va_list va;
	va_start(va, fmt);
	vfprintf(stderr, fmt, va);
	va_end(va);
	fflush(stderr);
}

int ExportCommon::progressMax = 0;
int ExportCommon::progressCur = 0;

void ExportCommon::progressSteps(int steps) {
	progressMax += steps;
	exportInfo("$:$%d\n", progressMax);
}

void ExportCommon::progressStep(const char *title) {
	progressCur++;
	exportInfo("$:$%d:%d:%s\n", progressMax, progressCur, title);
}

char *ExportCommon::askString(const char *title, const char *question,
		const char *def, bool key, const char *uid) {
	exportInfo("?:?%c%s|%s|%s|%s\n", key ? 'K' : 'S', title, question, def,
			uid);
	char str[512];
	fgets(str, 511, stdin);
	int i = strlen(str) - 1;
	if (str[i] == '\n')
		str[i] = '\0';
	return strdup(str);
}
