#include "gfile.h"
#include "gfile_p.h"
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string>
#include <map>
#include <gpath.h>
#include <gpath_p.h>

const char* g_pathForFile(const char* filename)
{
    return gpath_transform(filename);
}

const char* getDocumentsDirectory()
{
    return gpath_getDrivePath(1);
}

const char* getTemporaryDirectory()
{
    return gpath_getDrivePath(2);
}

const char* getResourceDirectory()
{
    return gpath_getDrivePath(0);
}

void setDocumentsDirectory(const char* documentsDirectory)
{
    return gpath_setDrivePath(1, documentsDirectory);
}

void setTemporaryDirectory(const char* temporaryDirectory)
{
    return gpath_setDrivePath(2, temporaryDirectory);
}

void setResourceDirectory(const char* resourceDirectory)
{
    return gpath_setDrivePath(0, resourceDirectory);
}

const char* pathForFileEx(const char* dir, const char* filename)
{
    return gpath_join(dir, filename);
}

FileType getFileType(const char* filename)
{
    switch (gpath_getPathDrive(filename))
    {
    case GPATH_ABSOLUTE:
        return eAbsoluteFile;
    case 0:
        return eResourceFile;
    case 1:
        return eDocumentFile;
    case 2:
        return eTemporaryFile;
    }

    return eResourceFile;
}
