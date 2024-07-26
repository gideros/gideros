#ifndef GDREXPORT_EXPORTLUA_H_
#define GDREXPORT_EXPORTLUA_H_

#include "exportcontext.h"
#include "ExportScript.h"

void ExportLUA_Init(ExportContext *ctx);
void ExportLUA_Cleanup(ExportContext *ctx);
bool ExportLUA_CallFile(ExportContext *ctx,ExportScript *scriptCtx,const char *fn);
bool ExportLUA_CallCode(ExportContext *ctx,ExportScript *scriptCtx,const char *code);
void ExportLUA_DonePlugins(ExportContext *ctx);

#endif
