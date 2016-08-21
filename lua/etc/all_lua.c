/*
* all.c -- Lua core, libraries and interpreter in a single file
*/

#define luaall_c


#include <gstdio.h>

#undef stdin
#undef stdout
#undef stderr
#undef getc
#undef ferror
#undef clearerr
#undef feof

#define FILE G_FILE
#define feof g_feof
#define fread g_fread
#define stdin g_stdin
#define stdout g_stdout
#define stderr g_stderr
#define fputs g_fputs
#define fopen g_fopen
#define getc g_getc
#define freopen g_freopen
#define ungetc g_ungetc
#define ferror g_ferror
#define fclose g_fclose
#define fprintf g_fprintf
#define fgets g_fgets
#define tmpfile g_tmpfile
#define fscanf g_fscanf
#define clearerr g_clearerr
#define fwrite g_fwrite
#define fseek g_fseek
#define ftell g_ftell
#define setvbuf g_setvbuf
#define fflush g_fflush

#include "lapi.c"
#include "lcode.c"
#include "ldebug.c"
#include "ldo.c"
#include "ldump.c"
#include "lfunc.c"
#include "lgc.c"
#include "llex.c"
#include "lmem.c"
#include "lobject.c"
#include "lopcodes.c"
#include "lparser.c"
#include "lstate.c"
#include "lstring.c"
#include "ltable.c"
#include "ltm.c"
#include "lundump.c"
#include "lvm.c"
#include "lzio.c"

#include "lauxlib.c"
#include "lbaselib.c"
#include "ldblib.c"
#include "liolib.c"
#include "linit.c"
#include "lmathlib.c"
#include "loadlib.c"
#include "loslib.c"
#include "lstrlib.c"
#include "ltablib.c"
#include "lutf8lib.c"
#include "lint64.c"

//#include "lua.c"
