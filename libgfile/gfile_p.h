#ifndef GFILE_P_H
#define GFILE_P_H

#include "gfile.h"

enum FileType
{
	eDocumentFile,
	eResourceFile,
	eTemporaryFile,
	eAbsoluteFile,
};

GIDEROS_API const char* getDocumentsDirectory();
GIDEROS_API const char* getTemporaryDirectory();
GIDEROS_API const char* getResourceDirectory();

GIDEROS_API void setDocumentsDirectory(const char* documentsDirectory);
GIDEROS_API void setTemporaryDirectory(const char* temporaryDirectory);
GIDEROS_API void setResourceDirectory(const char* resourceDirectory);

GIDEROS_API const char* pathForFileEx(const char* dir, const char* filename);
GIDEROS_API FileType getFileType(const char* filename);

#endif
