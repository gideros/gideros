#ifndef GDREXPORT_EXPORTLUA_H_
#define GDREXPORT_EXPORTLUA_H_

#include "exportcontext.h"
#include "ExportXml.h"

void ExportLUA_Init(ExportContext *ctx);
void ExportLUA_Cleanup(ExportContext *ctx);
bool ExportLUA_CallFile(ExportContext *ctx,ExportXml *xml,const char *fn);
bool ExportLUA_CallCode(ExportContext *ctx,ExportXml *xml,const char *code);

#endif
