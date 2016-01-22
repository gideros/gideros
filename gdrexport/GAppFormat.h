/*
 * GAppFormat.h
 *
 *  Created on: 22 janv. 2016
 *      Author: Nico
 */

#ifndef GDREXPORT_GAPPFORMAT_H_
#define GDREXPORT_GAPPFORMAT_H_
#include "exportcontext.h"
#include <QString>

class GAppFormat {
public:
	static void buildGApp(QString gappfile_, ExportContext *ctx);
};

#endif /* GDREXPORT_GAPPFORMAT_H_ */
