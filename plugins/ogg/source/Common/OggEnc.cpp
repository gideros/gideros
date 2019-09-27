/*
 * OggDec.cpp
 *
 *  Created on: 20 sept. 2019
 *      Author: Nico
 */

#include "OggEnc.h"
#include <map>
#include <string>

static std::map<std::string,OggEncType> cmap;

void register_oggenc(const char *name,OggEncType codec) {
	cmap[name]=codec;
}

void unregister_oggenc(const char *name) {
	cmap.erase(name);
}

OggEnc *build_oggenc(const char *name) {
	std::map<std::string,OggEncType>::const_iterator it=cmap.find(name);
	if (it!=cmap.cend())
		return it->second.build();
	return NULL;
}
