#include <stdlib.h>
#include <stdio.h>
#include <gstdio.h>
#include <sys/types.h>
#include <fcntl.h>

static g_Vfs s_vfs = {NULL, NULL, NULL, NULL, NULL};

void g_setVfs(g_Vfs vfs)
{
    s_vfs = vfs;
}

int g_p_open(const char *pathname, int flags, ...)
{
#ifdef O_BINARY
    flags = flags | O_BINARY;
#endif

    return s_vfs.open(pathname, flags);
}

int g_p_close(int fd)
{
    return s_vfs.close(fd);
}

size_t g_p_read(int fd, void* buf, size_t count)
{
    return s_vfs.read(fd, buf, count);
}

size_t g_p_write(int fd, const void* buf, size_t count)
{
    return s_vfs.write(fd, buf, count);
}

off_t g_p_lseek(int fd, off_t offset, int whence)
{
    return s_vfs.lseek(fd, offset, whence);
}
