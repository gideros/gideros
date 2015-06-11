#include "ogl.h"
#include "glog.h"
#include <string.h>
#include "gtexture.h"

#ifdef WINSTORE
#include "dx11Shaders.h"
#else
#include "gl2Shaders.h"
#endif
static bool oglInitialized=false;

void oglInitialize(unsigned int sw,unsigned int sh)
{
    if (oglInitialized) return;
#ifdef WINSTORE
	ShaderEngine::Engine = new dx11ShaderEngine(sw, sh);
#else
	ShaderEngine::Engine = new ogl2ShaderEngine(sw, sh);
#endif
  oglInitialized=true;
  gtexture_set_engine(ShaderEngine::Engine);
}


void oglCleanup()
{
	if (oglInitialized)
	{
    oglInitialized=false;
    delete ShaderEngine::Engine;
    ShaderEngine::Engine=NULL;
	}
}
