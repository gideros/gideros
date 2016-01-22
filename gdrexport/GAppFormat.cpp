/*
 * GAppFormat.cpp
 *
 *  Created on: 22 janv. 2016
 *      Author: Nico
 */

#include "GAppFormat.h"
#include <bytebuffer.h>

#define BYTE_SWAP4(x) \
    (((x & 0xFF000000) >> 24) | \
     ((x & 0x00FF0000) >> 8)  | \
     ((x & 0x0000FF00) << 8)  | \
     ((x & 0x000000FF) << 24))

#define BYTE_SWAP2(x) \
    (((x & 0xFF00) >> 8) | \
     ((x & 0x00FF) << 8))

quint16 _htons(quint16 x) {
	if (QSysInfo::ByteOrder == QSysInfo::BigEndian) {
		return x;
	} else {
		return BYTE_SWAP2(x);
	}
}

quint16 _ntohs(quint16 x) {
	if (QSysInfo::ByteOrder == QSysInfo::BigEndian) {
		return x;
	} else {
		return BYTE_SWAP2(x);
	}
}

quint32 _htonl(quint32 x) {
	if (QSysInfo::ByteOrder == QSysInfo::BigEndian) {
		return x;
	} else {
		return BYTE_SWAP4(x);
	}
}

quint32 _ntohl(quint32 x) {
	if (QSysInfo::ByteOrder == QSysInfo::BigEndian) {
		return x;
	} else {
		return BYTE_SWAP4(x);
	}
}

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
