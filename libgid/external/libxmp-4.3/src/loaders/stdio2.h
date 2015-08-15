#ifndef _STDIO2_H_
#define _STDIO2_H_

#include <gstdio.h>

#undef stdin
#undef stdout
#undef stderr
#undef getc
#undef putc
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
#define putc g_putc
#define fgetc g_fgetc
#define fputc g_fputc
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

#endif