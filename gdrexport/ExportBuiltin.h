/*
 * ExportBuiltin.h
 *
 *  Created on: 22 janv. 2016
 *      Author: Nico
 */

#ifndef GDREXPORT_EXPORTBUILTIN_H_
#define GDREXPORT_EXPORTBUILTIN_H_

#include "exportcontext.h"

class ExportBuiltin {
	static void exportAllAssetsFiles(ExportContext *ctx);
	static void fillTargetReplacements(ExportContext *ctx);
	static void prepareAssetFolder(ExportContext *ctx);
public:
	static void doExport(ExportContext *ctx);
};

#endif /* GDREXPORT_EXPORTBUILTIN_H_ */
