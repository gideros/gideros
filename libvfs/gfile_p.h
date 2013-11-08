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

G_API const char* getDocumentsDirectory();
G_API const char* getTemporaryDirectory();
G_API const char* getResourceDirectory();

G_API void setDocumentsDirectory(const char* documentsDirectory);
G_API void setTemporaryDirectory(const char* temporaryDirectory);
G_API void setResourceDirectory(const char* resourceDirectory);

G_API const char* pathForFileEx(const char* dir, const char* filename);
G_API FileType getFileType(const char* filename);

#endif
