#ifndef _GPATH_H_
#define _GPATH_H_

#include <gexport.h>
#include <gstdio.h>
/* path type */
#define GPATH_ABSOLUTE -1

/* drive flags */
#define GPATH_RO 1
#define GPATH_RW 2
#define GPATH_REAL 4

#ifdef __cplusplus
extern "C" {
#endif

G_API void gpath_init();
G_API void gpath_cleanup();

G_API void gpath_setDrivePath(int id, const char *path);
G_API void gpath_setDriveVfs(int id, const g_Vfs *vfs);
G_API void gpath_setDriveFlags(int id, int flags);
G_API void gpath_addDrivePrefix(int id, const char *prefix);

G_API void gpath_setAbsolutePathFlags(int flags);

G_API void gpath_setDefaultDrive(int id);
G_API int gpath_getDefaultDrive();

G_API const char *gpath_getDrivePath(int id);
G_API int gpath_getDriveFlags(int id);
G_API const g_Vfs *gpath_getDriveVfs(int id);

G_API int gpath_getPathDrive(const char *pathName);

G_API const char *gpath_transform(const char *pathName);
G_API const char *gpath_normalizeArchivePath(const char *pathname);


#ifdef __cplusplus
}
#endif

#endif
