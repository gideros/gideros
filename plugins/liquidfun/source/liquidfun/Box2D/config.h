
#if defined(_MSC_VER) || defined(__WATCOMC__) || defined(__MINGW32__) \
    || defined(_WIN32) || defined(_WIN32_WCE) || defined(__CYGWIN__)
#    include "config-windows.h"
#elif defined(__ANDROID__)
#    include "config-android.h"
#elif defined(__APPLE__)
#    include "config-darwin.h"
#elif defined(__linux__)
#    include "config-linux.h"
#else
#    include "config-other.h"
#endif
