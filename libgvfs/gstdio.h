#ifndef _GSTDIO_H_
#define _GSTDIO_H_


#include <gexport.h>

#include <stdarg.h>
#include <stdio.h>
#include <sys/types.h>

#ifdef __EMSCRIPTEN__
#define fpos_t off_t
#endif
#ifdef STRICT_LINUX
#define fpos_t off_t
#endif

/* stdio buffers */
struct g__sbuf {
	unsigned char *_base;
	int	_size;
};

/*
 * stdio state variables.
 *
 * The following always hold:
 *
 *	if (_flags&(__SLBF|__SWR)) == (__SLBF|__SWR),
 *		_lbfsize is -_bf._size, else _lbfsize is 0
 *	if _flags&__SRD, _w is 0
 *	if _flags&__SWR, _r is 0
 *
 * This ensures that the getc and putc macros (or inline functions) never
 * try to write or read from a file that is in `read' or `write' mode.
 * (Moreover, they can, and do, automatically switch from read mode to
 * write mode, and back, on "r+" and "w+" files.)
 *
 * _lbfsize is used only to make the inline line-buffered output stream
 * code as compact as possible.
 *
 * _ub, _up, and _ur are used when ungetc() pushes back more characters
 * than fit in the current _bf, or when ungetc() pushes back a character
 * that does not match the previous one in _bf.  When this happens,
 * _ub._base becomes non-nil (i.e., a stream has ungetc() data iff
 * _ub._base!=NULL) and _up and _ur save the current values of _p and _r.
 */
typedef	struct g__sFILE {
	unsigned char *_p;	/* current position in (some) buffer */
	int	_r;		/* read space left for getc() */
	int	_w;		/* write space left for putc() */
	short	_flags;		/* flags, below; this FILE is free if 0 */
	short	_file;		/* fileno, if Unix descriptor, else -1 */
    struct	g__sbuf _bf;	/* the buffer (at least 1 byte, if !NULL) */
	int	_lbfsize;	/* 0 or -_bf._size, for inline putc */

	/* operations */
	void	*_cookie;	/* cookie passed to io functions */
	int	(*_close)(void *);
	int	(*_read)(void *, char *, int);
	fpos_t	(*_seek)(void *, fpos_t, int);
	int	(*_write)(void *, const char *, int);

	/* extension data, to avoid further ABI breakage */
    struct	g__sbuf _ext;
	/* data for long sequences of ungetc() */
	unsigned char *_up;	/* saved _p when _p is doing ungetc data */
	int	_ur;		/* saved _r when _r is counting ungetc data */

	/* tricks to meet minimum requirements even when malloc() fails */
	unsigned char _ubuf[3];	/* guarantee an ungetc() buffer */
	unsigned char _nbuf[1];	/* guarantee a getc() buffer */

	/* separate buffer for fgetln() when line crosses buffer boundary */
    struct	g__sbuf _lb;	/* buffer for fgetln() */

	/* Unix stdio files get aligned to block boundaries on fseek() */
	int	_blksize;	/* stat.st_blksize (may be != _bf._size) */
	fpos_t	_offset;	/* current lseek offset */
} G_FILE;

#ifdef __cplusplus
extern "C" {
#endif

G_API void	 g_flockfile(G_FILE *);
G_API int	 g_ftrylockfile(G_FILE *);
G_API void	 g_funlockfile(G_FILE *);

G_API int	 g_fflush(G_FILE *);
G_API G_FILE	*g_fopen(const char *, const char *);
G_API G_FILE	*g_freopen(const char *, const char *, G_FILE *);
G_API int	 g_fclose(G_FILE *);
G_API size_t	 g_fread(void *, size_t, size_t, G_FILE *);
G_API size_t	 g_fwrite(const void *, size_t, size_t, G_FILE *);
G_API int	 g_fseek(G_FILE *, long, int);
G_API long	 g_ftell(G_FILE *);
G_API int	 g_feof(G_FILE *);
G_API int	 g_ferror(G_FILE *);
G_API void	 g_clearerr(G_FILE *);
G_API int	 g_fgetc(G_FILE *);
G_API int	 g_getc(G_FILE *);
G_API int	 g_fputc(int c, G_FILE *fp);
G_API int	 g_putc(int c, G_FILE *fp);
G_API char	*g_fgets(char *, int, G_FILE *);
G_API int	 g_ungetc(int, G_FILE *);
G_API int	 g_fscanf(G_FILE *, const char *, ...);
G_API int	 g_vfscanf(G_FILE *, const char *, va_list);
G_API int	 g_fprintf(G_FILE *, const char *, ...);
G_API int	 g_vfprintf(G_FILE *, const char *, va_list);
G_API int	 g_setvbuf(G_FILE *, char *, int, size_t);
G_API G_FILE	*g_tmpfile(void);
G_API int	 g_fputs(const char *, G_FILE *);

extern G_API G_FILE g__sF[];

#define	g_stdin (&g__sF[0])
#define	g_stdout (&g__sF[1])
#define	g_stderr (&g__sF[2])

#ifdef __cplusplus
}
#endif

typedef struct g_Vfs
{
    int (*open)(const char *pathname, int flags);
    int (*close)(int fd);
    size_t (*read)(int fd, void* buf, size_t count);
    size_t (*write)(int fd, const void* buf, size_t count);
    off_t (*lseek)(int fd, off_t offset, int whence);
} g_Vfs;

#ifdef __cplusplus
extern "C" {
#endif

G_API void g_setVfs(g_Vfs vfs);

#ifdef __cplusplus
}
#endif

#endif
