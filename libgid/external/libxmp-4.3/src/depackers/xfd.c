/* xfdmaster.library decruncher for XMP
 * Copyright (C) 2007 Chris Young
 *
 * This file is part of the Extended Module Player and is distributed
 * under the terms of the GNU Lesser General Public License. See COPYING.LIB
 * for more information.
 */

#ifdef __SUNPRO_C
#pragma error_messages (off,E_EMPTY_TRANSLATION_UNIT)
#endif

#ifdef AMIGA
#define __USE_INLINE__
#include <proto/exec.h>
#include <proto/xfdmaster.h>
#include <exec/types.h>
#include "stdio2.h"
#include <sys/stat.h>

struct local_data {
	struct Library *xfdMasterBase;
#ifdef __amigaos4__
	struct xfdMasterIFace *IxfdMaster;
	struct ExecIFace *IExec;
	// = (struct ExecIFace *)(*(struct ExecBase **)4)->MainInterface;
#endif
};

struct xfdBufferInfo *open_xfd(struct local_data *data)
{
#ifdef __amigaos4__
	data->IExec = (struct ExecIFace *)(*(struct ExecBase **)4)->MainInterface;
#endif

	if(data->xfdMasterBase = OpenLibrary("xfdmaster.library",38))
	{
#ifdef __amigaos4__
		if(data->IxfdMaster = (struct xfdMasterIFace *)GetInterface(data->xfdMasterBase,"main",1,NULL))
		{
#endif
			return(struct xfdBufferInfo *)xfdAllocObject(XFDOBJ_BUFFERINFO);
#ifdef __amigaos4__
		}
#endif
	}
	close_xfd(NULL);
	return NULL;
}

void close_xfd(struct xfdBufferInfo *xfdobj, struct local_data *data)
{
	if(xfdobj)
	{
		xfdFreeObject((APTR)xfdobj);
		xfdobj=NULL;
	}
#ifdef __amigaos4__
	if(data->IxfdMaster)
	{
		DropInterface((struct Interface *)data->IxfdMaster);
		data->IxfdMaster=NULL;
	}
#endif
	if(data->xfdMasterBase)
	{
		CloseLibrary(data->xfdMasterBase);
		data->xfdMasterBase=NULL;
	}
}

static char *_test_xfd(unsigned char *buffer, int length)
{
	char *ret = NULL;
	struct xfdBufferInfo *xfdobj;
	struct local_data data;

	if(xfdobj=open_xfd(&data))
	{
		xfdobj->xfdbi_SourceBuffer = buffer;
		xfdobj->xfdbi_SourceBufLen = length;
		xfdobj->xfdbi_Flags = XFDFB_RECOGTARGETLEN | XFDFB_RECOGEXTERN;

		if(xfdRecogBuffer(xfdobj))
		{
			ret = xfdobj->xfdbi_PackerName;
		}
		close_xfd(xfdobj, &data);
	}
	return(ret);
}

static int test_xfd(unsigned char *b)
{
	return _test_xfd(b, 1024) != NULL;
}

static int decrunch_xfd(FILE *f1, FILE *f2)
{
    struct xfdBufferInfo *xfdobj;
    uint8 *packed;
    int plen,ret=-1;
    struct stat st;
    struct local_data data;

    if (f2 == NULL)
	return -1;

    fstat(fileno(f1), &st);
    plen = st.st_size;

    packed = AllocVec(plen,MEMF_CLEAR);
    if (!packed) return -1;

    fread(packed,plen,1,f1);

	if(xfdobj=open_xfd(&data))
	{
		xfdobj->xfdbi_SourceBufLen = plen;
		xfdobj->xfdbi_SourceBuffer = packed;
		xfdobj->xfdbi_Flags = XFDFF_RECOGEXTERN | XFDFF_RECOGTARGETLEN;
		/* xfdobj->xfdbi_PackerFlags = XFDPFF_RECOGLEN; */
		if(xfdRecogBuffer(xfdobj))
		{
			xfdobj->xfdbi_TargetBufMemType = MEMF_ANY;
			if(xfdDecrunchBuffer(xfdobj))
			{
				if(fwrite(xfdobj->xfdbi_TargetBuffer,1,xfdobj->xfdbi_TargetBufSaveLen,f2) == xfdobj->xfdbi_TargetBufSaveLen) ret=0;
				FreeMem(xfdobj->xfdbi_TargetBuffer,xfdobj->xfdbi_TargetBufLen);
			}
			else
			{
				ret=-1;
			}
		}
		close_xfd(xfdobj, &data);
	}
	FreeVec(packed);
	return(ret);
}

struct depacker xfd_depacker = {
	test_xfd,
	decrunch_xfd
};

#endif /* AMIGA */
