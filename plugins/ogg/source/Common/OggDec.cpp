/*
 * OggDec.cpp
 *
 *  Created on: 20 sept. 2019
 *      Author: Nico
 */

#include "OggDec.h"
#include <map>
#include <string>

static std::map<std::string,OggDecType> cmap;

void register_oggdec(const char *name,OggDecType codec) {
	cmap[name]=codec;
}

void unregister_oggdec(const char *name) {
	cmap.erase(name);
}

OggDec *probe_oggdec(ogg_packet *op,int type) {
	for (std::map<std::string,OggDecType>::const_iterator it=cmap.cbegin();it!=cmap.cend();it++) {
		OggDec *res=NULL;
		if ((it->second.type==type)&&((res=it->second.probe(op))!=NULL)) return res;
	}
	return NULL;
}
