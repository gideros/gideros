/*
 * ExportCommon.h
 *
 *  Created on: 22 janv. 2016
 *      Author: Nico
 */

#ifndef GDREXPORT_EXPORTCOMMON_H_
#define GDREXPORT_EXPORTCOMMON_H_

#include "exportcontext.h"
class ExportCommon {
	static int progressMax;
	static int progressCur;
public:
    static void copyTemplate(QString templatePath,QString templateDest,ExportContext *ctx, bool isPlugin,QStringList include,QStringList exclude);
	static void exportAssets(ExportContext *ctx, bool compileLua);
	static void exportPropertiesBin(ExportContext *ctx);
	static void exportLuafilesTxt(ExportContext *ctx);
	static void exportAllfilesTxt(ExportContext *ctx);
	static bool initPlugins(ExportContext *ctx);
	static void requestPlugin(ExportContext *ctx,QString name);
	static bool applyPlugins(ExportContext *ctx);
    static void resizeImage(QImage *image, int width, int height, QString output, int quality = -1,bool withAlpha=true,QColor fill=QColor("transparent"), int mode = 0, bool paletted=false);
    static bool appIcon(ExportContext *ctx,int width,int height,QString output,bool withAlpha=true, bool paletted=false);
    static bool tvIcon(ExportContext *ctx, int width, int height, QString output,bool withAlpha=true);
    static bool splashHImage(ExportContext *ctx, int width, int height, QString output,bool withAlpha=true);
    static bool splashVImage(ExportContext *ctx, int width, int height, QString output,bool withAlpha=true);
    static bool download(ExportContext *ctx,QString url,QString to);
    static bool unzip(ExportContext *ctx, QString file,QString dest);
	static void exportInfo(const char *fmt,...);
	static void exportError(const char *fmt,...);
	static char *askString(const char *title, const char *question, const char *def, bool key=false, const char *uid="");
	static void progressSteps(int steps);
	static void progressStep(const char *title);
};

#endif /* GDREXPORT_EXPORTCOMMON_H_ */
