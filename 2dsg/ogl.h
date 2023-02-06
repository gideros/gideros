#ifndef OGL_H_INCLUDED
#define OGL_H_INCLUDED

#include "Shaders.h"

#define PREMULTIPLIED_ALPHA 1

// remove any macros which will clash with C++ std::max, std::min
#ifdef WINSTORE
#undef min
#undef max
#endif

#ifndef M_PI
#define M_PI       3.14159265358979323846
#endif


void oglInitialize(unsigned int sw,unsigned int sh);
void oglCleanup();

//Renderer SYNC macros
#define RENDER_START()
#define RENDER_END()
#define RENDER_LOCK()
#define RENDER_UNLOCK()
#define RENDER_DO(f) f()

#endif

