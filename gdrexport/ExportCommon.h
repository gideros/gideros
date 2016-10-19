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
	static bool applyPlugins(ExportContext *ctx);
    static void resizeImage(QImage *image, int width, int height, QString output, int quality = -1);
	static bool appIcon(ExportContext *ctx,int width,int height,QString output);
    static bool tvIcon(ExportContext *ctx, int width, int height, QString output);
    static bool splashHImage(ExportContext *ctx, int width, int height, QString output);
    static bool splashVImage(ExportContext *ctx, int width, int height, QString output);
    static bool download(ExportContext *ctx,QString url,QString to);
    static bool unzip(ExportContext *ctx, QString file,QString dest);
	static void exportInfo(const char *fmt,...);
	static void exportError(const char *fmt,...);
	static char *askString(const char *title, const char *question, const char *def);
	static void progressSteps(int steps);
	static void progressStep(const char *title);
};

#endif /* GDREXPORT_EXPORTCOMMON_H_ */
