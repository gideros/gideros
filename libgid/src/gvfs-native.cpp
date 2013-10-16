#include <stdio.h>
#include <gstdio.h>

#include <unistd.h>

#include <fcntl.h>
#include <errno.h>

#include <gpath.h>

#include <map>

#include <gvfs-native.h>

#include <string.h>
#include <ctype.h>

#ifdef _WIN32
#define strcasecmp stricmp
#endif

struct FileInfo
{
    int encrypt;    // 0 no encryption, 1:encrypt code, 2:encrypt assets
};

static std::map<int, FileInfo> s_fileInfos;

static char s_codeKey[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static char s_assetsKey[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

static int s_open(const char *pathname, int flags)
{
    int drive = gpath_getPathDrive(pathname);

    if (drive != GPATH_ABSOLUTE)
    {
        if ((gpath_getDriveFlags(drive) & GPATH_RO) == GPATH_RO && (flags & O_ACCMODE) != O_RDONLY)
        {
            errno = EACCES;
            return -1;
        }
    }

    int fd = ::open(gpath_transform(pathname), flags, 0755);

    if (fd < 0)
        return fd;

    FileInfo fi = {0};

    if (drive == 0)
    {
        const char *ext = strrchr(pathname, '.');

        if (ext)
        {
            ext++;
            if (!strcasecmp(ext, "lua"))
            {
                fi.encrypt = 1;
            }
            else if (!strcasecmp(ext, "jpeg") ||
                     !strcasecmp(ext, "jpg") ||
                     !strcasecmp(ext, "png") ||
                     !strcasecmp(ext, "wav"))
            {
                fi.encrypt = 2;
            }
        }
    }

    s_fileInfos[fd] = fi;

    return fd;
}

static int s_close(int fd)
{
    std::map<int, FileInfo>::iterator iter;
    iter = s_fileInfos.find(fd);

    if (iter == s_fileInfos.end()) /* sanity check */
    {
        errno = EBADF;
        return -1;
    }

    s_fileInfos.erase(fd);
    return close(fd);
}

static size_t readHelper(int fd, void* buf, size_t count)
{
    std::map<int, FileInfo>::iterator iter;
    iter = s_fileInfos.find(fd);

    if (iter == s_fileInfos.end()) /* sanity check */
    {
        errno = EBADF;
        return -1;
    }

    return ::read(fd, buf, count);
}

static size_t s_write(int fd, const void* buf, size_t count)
{
    std::map<int, FileInfo>::iterator iter;
    iter = s_fileInfos.find(fd);

    if (iter == s_fileInfos.end()) /* sanity check */
    {
        errno = EBADF;
        return -1;
    }

    return ::write(fd, buf, count);
}

static off_t s_lseek(int fd, off_t offset, int whence)
{
    std::map<int, FileInfo>::iterator iter;
    iter = s_fileInfos.find(fd);

    if (iter == s_fileInfos.end()) /* sanity check */
    {
        errno = EBADF;
        return -1;
    }

    return ::lseek(fd, offset, whence);
}

static size_t s_read(int fd, void* buf, size_t count)
{
    std::map<int, FileInfo>::iterator iter;
    iter = s_fileInfos.find(fd);

    if (iter == s_fileInfos.end()) /* sanity check */
    {
        errno = EBADF;
        return -1;
    }

    size_t size;

    char *key;
    switch (iter->second.encrypt)
    {
    case 0:
        key = NULL;
        break;
    case 1:
        key = s_codeKey;
        break;
    case 2:
        key = s_assetsKey;
        break;
    }

    if (key)
    {
        off_t curr = s_lseek(fd, 0, SEEK_CUR);
        size = readHelper(fd, buf, count);

        if (curr != (off_t)-1 && size != (size_t)-1)
            for (size_t i = 0; i < size; ++i)
                ((char*)buf)[i] ^= key[(curr + i) % 16];
    }
    else
    {
        size = readHelper(fd, buf, count);
    }

    return size;
}


extern "C" {

void gvfs_init()
{
    g_Vfs vfs =
    {
        s_open,
        s_close,
        s_read,
        s_write,
        s_lseek,
    };

    g_setVfs(vfs);
}

void gvfs_cleanup()
{
    std::map<int, FileInfo>::iterator iter, e = s_fileInfos.end();
    for (iter = s_fileInfos.begin(); iter != e; ++iter)
        ::close(iter->first);

    s_fileInfos.clear();

    g_Vfs vfs =
    {
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
    };

    g_setVfs(vfs);
}

void gvfs_setCodeKey(const char key[16])
{
    memcpy(s_codeKey, key, 16);
}

void gvfs_setAssetsKey(const char key[16])
{
    memcpy(s_assetsKey, key, 16);
}

}
