/*
 * MacOSXExport.h
 *
 *  Created on: 22 janv. 2016
 *      Author: Nico
 */

#ifndef GDREXPORT_MACOSXEXPORT_H_
#define GDREXPORT_MACOSXEXPORT_H_

#include "exportcontext.h"
class MacOSXExport {
public:
	static void CodeSignMacOSX(ExportContext *ctx);
};

#endif /* GDREXPORT_MACOSXEXPORT_H_ */
