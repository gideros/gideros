#include <glog.h>

#ifdef QT_CORE_LIB
#include <QDebug>
#endif

#ifdef __ANDROID__
#include <android/log.h>
#endif

#include <stdarg.h>
#include <stdio.h>

static int s_level = GLOG_DEBUG;

static void log(const char *buffer)
{
#if defined(QT_CORE_LIB)
    qDebug() << buffer;
#elif defined(__ANDROID__)
    __android_log_print(ANDROID_LOG_DEBUG, "Gideros", "%s", buffer);
#else
    printf("%s\n", buffer);
#endif
}

extern "C" {

G_API void glog_v(const char *format, ...)
{
    if (s_level > GLOG_VERBOSE)
        return;

    char buffer[1024];
    va_list args;
    va_start (args, format);
    vsprintf (buffer, format, args);
    va_end (args);

    log(buffer);
}

G_API void glog_d(const char *format, ...)
{
    if (s_level > GLOG_DEBUG)
        return;

    char buffer[1024];
    va_list args;
    va_start (args, format);
    vsprintf (buffer, format, args);
    va_end (args);

    log(buffer);
}

G_API void glog_i(const char *format, ...)
{
    if (s_level > GLOG_INFO)
        return;

    char buffer[1024];
    va_list args;
    va_start (args, format);
    vsprintf (buffer, format, args);
    va_end (args);

    log(buffer);
}

G_API void glog_w(const char *format, ...)
{
    if (s_level > GLOG_WARNING)
        return;

    char buffer[1024];
    va_list args;
    va_start (args, format);
    vsprintf (buffer, format, args);
    va_end (args);

    log(buffer);
}

G_API void glog_e(const char *format, ...)
{
    if (s_level > GLOG_ERROR)
        return;

    char buffer[1024];
    va_list args;
    va_start (args, format);
    vsprintf (buffer, format, args);
    va_end (args);

    log(buffer);
}

G_API void glog_setLevel(int level)
{
    s_level = level;
}

G_API int glog_getLevel()
{
    return s_level;
}

}
