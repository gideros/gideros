#ifndef _DEFINES_H_
#define _DEFINES_H_

#ifdef __EMSCRIPTEN__
#define fpos_t off_t
#endif
#ifdef STRICT_LINUX
#define fpos_t off_t
#endif

#define __sbuf g__sbuf
#define __sFILE g__sFILE
#define FILE G_FILE

#define flockfile g_flockfile
#define ftrylockfile g_ftrylockfile
#define funlockfile g_funlockfile

#define fflush g_fflush
#define fopen g_fopen
#define freopen g_freopen
#define fclose g_fclose
#define fread g_fread
#define fwrite g_fwrite
#define fseek g_fseek
#define fseeko g_fseeko
#define ftell g_ftell
#define ftello g_ftello
#undef feof
#define feof g_feof
#undef ferror
#define ferror g_ferror
#undef clearerr
#define clearerr g_clearerr
#undef getc
#define getc g_getc
#define fgetc g_fgetc
#undef putc
#define putc g_putc
#define fputc g_fputc
#undef getc_unlocked
#define getc_unlocked g_getc_unlocked
#undef putc_unlocked
#define putc_unlocked g_putc_unlocked
#define __srget g__srget
#define fgets g_fgets
#define ungetc g_ungetc
#define fscanf g_fscanf
#define vfscanf g_vfscanf
#define fprintf g_fprintf
#define vfprintf g_vfprintf
#define setvbuf g_setvbuf
#define tmpfile g_tmpfile
#define fputs g_fputs
#define __swbuf g__swbuf
#define __sputc g__sputc

#define __sF g__sF

#define __sdidinit g__sdidinit

#define __isthreaded g__isthreaded

#define __CONCAT(x,y) x ## y

#ifndef ALIGNBYTES
#define ALIGNBYTES  3
#endif

#ifndef ALIGN
#define ALIGN(p)    (((unsigned int)(p) + ALIGNBYTES) &~ ALIGNBYTES)
#endif

#define	__SLBF	0x0001		/* line buffered */
#define	__SNBF	0x0002		/* unbuffered */
#define	__SRD	0x0004		/* OK to read */
#define	__SWR	0x0008		/* OK to write */
    /* RD and WR are never simultaneously asserted */
#define	__SRW	0x0010		/* open for reading & writing */
#define	__SEOF	0x0020		/* found EOF */
#define	__SERR	0x0040		/* found error */
#define	__SMBF	0x0080		/* _buf is from malloc */
#define	__SAPP	0x0100		/* fdopen()ed in append mode */
#define	__SSTR	0x0200		/* this is an sprintf/snprintf string */
#define	__SOPT	0x0400		/* do fseek() optimisation */
#define	__SNPT	0x0800		/* do not do fseek() optimisation */
#define	__SOFF	0x1000		/* set iff _offset is in fact correct */
#define	__SMOD	0x2000		/* true => fgetln modified _p text */
#define	__SALC	0x4000		/* allocate string space dynamically */
#define __SIGN	0x8000		/* ignore this file in _fwalk */


#define open g_p_open
#define close g_p_close
#define read g_p_read
#define write g_p_write
#define lseek g_p_lseek

extern int open(const char *pathname, int flags, ...);
extern int close(int fd);
extern size_t read(int fd, void* buf, size_t count);
extern size_t write(int fd, const void* buf, size_t count);
extern off_t lseek(int fd, off_t offset, int whence);

#endif
