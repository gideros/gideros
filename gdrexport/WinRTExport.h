/*
 * WinRTExport.h
 *
 *  Created on: 22 janv. 2016
 *      Author: Nico
 */

#ifndef GDREXPORT_WINRTEXPORT_H_
#define GDREXPORT_WINRTEXPORT_H_

#include "exportcontext.h"
class WinRTExport {
public:
	static void updateWinRTProject(QString projfile,ExportContext *ctx);
    static QByteArray makeIconFile(QImage *image, QColor fill, int mode);
};

#endif /* GDREXPORT_WINRTEXPORT_H_ */
