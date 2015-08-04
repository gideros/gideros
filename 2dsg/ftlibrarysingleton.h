#ifndef FT_LIBRARY_SINGLETON
#define FT_LIBRARY_SINGLETON

#include <ft2build.h>
#include FT_FREETYPE_H

class FT_Library_Singleton
{
public:
    FT_Library_Singleton();
    ~FT_Library_Singleton();

    static FT_Library &instance();

private:
    FT_Library library_;
};

#endif
