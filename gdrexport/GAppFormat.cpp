/*
 * GAppFormat.cpp
 *
 *  Created on: 22 janv. 2016
 *      Author: Nico
 */

#include "GAppFormat.h"
#include <bytebuffer.h>
#include "cendian.h"

void GAppFormat::buildGApp(QString gappfile_, ExportContext *ctx) {
	QFile file(gappfile_);
	if (file.open(QIODevice::WriteOnly)) {

		ByteBuffer buffer;
		quint32 offset = 0;
		quint32 cbuf;
		char cpbuf[4096];
		for (int k = 0; k < ctx->allfiles.size(); k++) {
			buffer.append(ctx->allfiles[k].toStdString());
			cbuf = _htonl(offset);
			buffer.append((unsigned char *) &cbuf, 4);
			QFile src(ctx->allfiles_abs[k]);
			src.open(QIODevice::ReadOnly);
			int size = 0;
			while (true) {
				int rd = src.read(cpbuf, sizeof(cpbuf));
				if (rd <= 0)
					break;
				size += rd;
				file.write(cpbuf, rd);
				if (rd < sizeof(cpbuf))
					break;
			}
			src.close();
			cbuf = _htonl(size);
			buffer.append((unsigned char *) &cbuf, 4);
			offset += size;
		}
		buffer.append(""); //End of file list marker, i.e. empty filename
		if (offset & 7)
			offset += file.write(cpbuf, 8 - (offset & 7)); // Align structure to 8 byte boundary
		cbuf = _htonl(offset); //File list offset from beginning of package
		buffer.append((unsigned char *) &cbuf, 4);
		cbuf = _htonl(0); //Version
		buffer.append((unsigned char *) &cbuf, 4);
		buffer.append("GiDeRoS"); //Package marker
		file.write(buffer.data(), buffer.size());
	}
	file.close();
}
