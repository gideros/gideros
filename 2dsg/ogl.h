#ifndef OGL_H_INCLUDED
#define OGL_H_INCLUDED

#include "Shaders.h"

#define PREMULTIPLIED_ALPHA 1

// remove any macros which will clash with C++ std::max, std::min
#ifdef WINSTORE
#undef min
#undef max

#ifndef M_PI
#define M_PI 3.141592654
#endif
#endif


void oglInitialize(unsigned int sw,unsigned int sh);
void oglCleanup();

#endif

