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
	static void copyTemplate(QString templatePath,ExportContext *ctx);
	static void exportAssets(ExportContext *ctx, bool compileLua);
	static void exportPropertiesBin(ExportContext *ctx);
	static void exportLuafilesTxt(ExportContext *ctx);
	static void exportAllfilesTxt(ExportContext *ctx);
	static bool applyPlugins(ExportContext *ctx);
	static bool appIcon(ExportContext *ctx,int width,int height,QString output);
	static void exportInfo(const char *fmt,...);
	static void progressSteps(int steps);
	static void progressStep(const char *title);
};

#endif /* GDREXPORT_EXPORTCOMMON_H_ */
