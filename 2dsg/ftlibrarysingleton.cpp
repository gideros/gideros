#include "ftlibrarysingleton.h"

FT_Library_Singleton::FT_Library_Singleton()
{
    FT_Init_FreeType(&library_);
}

FT_Library_Singleton::~FT_Library_Singleton()
{
    FT_Done_FreeType(library_);
}

FT_Library &FT_Library_Singleton::instance()
{
    static FT_Library_Singleton instance;
    return instance.library_;
}

