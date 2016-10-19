/*
 * cendian.h
 *
 *  Created on: 19 oct. 2016
 *      Author: Nico
 */

#ifndef GDREXPORT_CENDIAN_H_
#define GDREXPORT_CENDIAN_H_



#define BYTE_SWAP4(x) \
    (((x & 0xFF000000) >> 24) | \
     ((x & 0x00FF0000) >> 8)  | \
     ((x & 0x0000FF00) << 8)  | \
     ((x & 0x000000FF) << 24))

#define BYTE_SWAP2(x) \
    (((x & 0xFF00) >> 8) | \
     ((x & 0x00FF) << 8))

static inline quint16 _htons(quint16 x) {
	if (QSysInfo::ByteOrder == QSysInfo::BigEndian) {
		return x;
	} else {
		return BYTE_SWAP2(x);
	}
}

static inline quint16 _ntohs(quint16 x) {
	if (QSysInfo::ByteOrder == QSysInfo::BigEndian) {
		return x;
	} else {
		return BYTE_SWAP2(x);
	}
}

static inline quint32 _htonl(quint32 x) {
	if (QSysInfo::ByteOrder == QSysInfo::BigEndian) {
		return x;
	} else {
		return BYTE_SWAP4(x);
	}
}

static inline quint32 _ntohl(quint32 x) {
	if (QSysInfo::ByteOrder == QSysInfo::BigEndian) {
		return x;
	} else {
		return BYTE_SWAP4(x);
	}
}

static inline quint16 _htobes(quint16 x) {
	if (QSysInfo::ByteOrder == QSysInfo::BigEndian) {
		return x;
	} else {
		return BYTE_SWAP2(x);
	}
}

static inline quint16 _betohs(quint16 x) {
	if (QSysInfo::ByteOrder == QSysInfo::BigEndian) {
		return x;
	} else {
		return BYTE_SWAP2(x);
	}
}

static inline quint32 _htobel(quint32 x) {
	if (QSysInfo::ByteOrder == QSysInfo::BigEndian) {
		return x;
	} else {
		return BYTE_SWAP4(x);
	}
}

static inline quint32 _betohl(quint32 x) {
	if (QSysInfo::ByteOrder == QSysInfo::BigEndian) {
		return x;
	} else {
		return BYTE_SWAP4(x);
	}
}

static inline quint16 _htoles(quint16 x) {
	if (QSysInfo::ByteOrder == QSysInfo::LittleEndian) {
		return x;
	} else {
		return BYTE_SWAP2(x);
	}
}

static inline quint16 _letohs(quint16 x) {
	if (QSysInfo::ByteOrder == QSysInfo::LittleEndian) {
		return x;
	} else {
		return BYTE_SWAP2(x);
	}
}

static inline quint32 _htolel(quint32 x) {
	if (QSysInfo::ByteOrder == QSysInfo::LittleEndian) {
		return x;
	} else {
		return BYTE_SWAP4(x);
	}
}

static inline quint32 _letohl(quint32 x) {
	if (QSysInfo::ByteOrder == QSysInfo::LittleEndian) {
		return x;
	} else {
		return BYTE_SWAP4(x);
	}
}


#endif /* GDREXPORT_CENDIAN_H_ */
