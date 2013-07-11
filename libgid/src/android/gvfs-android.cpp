#include <gvfs-android.h>

#include <stdio.h>
#include <gstdio.h>
#include <string>
#include <map>

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

#include <fcntl.h>
#include <string.h>
#include <errno.h>

#include <gpath.h>

#include <pystring.h>

struct FileInfo
{
	int zipFile;
    size_t startOffset;
    size_t length;
    bool encrypted;
};

static std::vector<std::string> s_zipFiles;
static std::map<std::string, FileInfo> s_files;
static std::map<int, FileInfo> s_fileInfos;
static bool s_playerModeEnabled = false;

static char s_key[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

static int s_open(const char *pathname, int flags)
{
	int drive = gpath_getPathDrive(pathname);
	
    if (s_playerModeEnabled == true || drive != 0)
    {
        int fd = ::open(gpath_transform(pathname), flags, 0755);

        if (fd < 0)
            return fd;

        FileInfo fi = {-1, (size_t)-1, (size_t)-1, false};
        s_fileInfos[fd] = fi;

        return fd;
    }

	std::string normpathname = pystring::os::path::normpath(gpath_transform(pathname));
	pathname = normpathname.c_str();

    std::map<std::string, FileInfo>::iterator iter;
    iter = s_files.find(pathname);

    if (iter == s_files.end())
    {
        errno = ENOENT;
        return -1;
    }

    if ((flags & O_ACCMODE) != O_RDONLY)
    {
        errno = EACCES;
        return -1;
    }

    int fd = ::open(s_zipFiles[iter->second.zipFile].c_str(), flags, 0755);

    if (fd < 0)
        return fd;

    FileInfo fi = iter->second;

    if (drive == 0)
    {
        char *ext = strrchr(pathname, '.');

        if (ext &&
            tolower(ext[1]) == 'l' &&
            tolower(ext[2]) == 'u' &&
            tolower(ext[3]) == 'a')
        {
            fi.encrypted = true;
        }
    }

    ::lseek(fd, iter->second.startOffset, SEEK_SET);

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

    if (iter->second.startOffset == (size_t)-1 && iter->second.length == (size_t)-1)
        return ::read(fd, buf, count);

    size_t endOffset = iter->second.startOffset + iter->second.length;

    size_t curr = ::lseek(fd, 0, SEEK_CUR);

    if (curr < iter->second.startOffset || curr >= endOffset)
        return 0;

    size_t rem = endOffset - curr;

    return ::read(fd, buf, std::min(rem, count));
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

    if (iter->second.startOffset != (size_t)-1 || iter->second.length != (size_t)-1) /* sanity check */
    {
        errno = EACCES;
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

    if (iter->second.startOffset == (size_t)-1 && iter->second.length == (size_t)-1)
        return ::lseek(fd, offset, whence);

    size_t startOffset = iter->second.startOffset;
    size_t endOffset = startOffset + iter->second.length;

    switch (whence)
    {
    case SEEK_SET:
    {
        off_t result = ::lseek(fd, startOffset + offset, SEEK_SET);
        return result - startOffset;
    }
    case SEEK_CUR:
    {
        off_t result = ::lseek(fd, offset, SEEK_CUR);
        return result - startOffset;
    }
    case SEEK_END:
    {
        off_t result = ::lseek(fd, endOffset + offset, SEEK_SET);
        return result - startOffset;
    }
    }

    errno = EINVAL;
    return -1;
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

    if (iter->second.encrypted)
    {
        off_t curr = s_lseek(fd, 0, SEEK_CUR);
        size = readHelper(fd, buf, count);

        if (curr != (off_t)-1 && size != (size_t)-1)
            for (size_t i = 0; i < size; ++i)
                ((char*)buf)[i] ^= s_key[(curr + i) % 16];
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

	s_zipFiles.clear();
	s_files.clear();
	s_fileInfos.clear();
	s_playerModeEnabled = false;

	static g_Vfs nullvfs =
	{
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
	};

	g_setVfs(nullvfs);
}

void gvfs_setPlayerModeEnabled(int playerMode)
{
    s_playerModeEnabled = playerMode;
}

int gvfs_isPlayerModeEnabled()
{
    return s_playerModeEnabled;
}

void gvfs_setZipFiles(const char *apkFile, const char *mainFile, const char *patchFile)
{
	s_zipFiles.clear();
    s_zipFiles.push_back(apkFile);
	s_zipFiles.push_back(mainFile);
	s_zipFiles.push_back(patchFile);
}

void gvfs_addFile(const char *pathname, int zipFile, size_t startOffset, size_t length)
{
    FileInfo f = {zipFile, startOffset, length, false};
    s_files[pathname] = f;
}

void gvfs_setEncryptionKey(const char key[16])
{
    memcpy(s_key, key, 16);
}

}
