#include "ogl.h"
#include "glog.h"
#include <string.h>
#include "gtexture.h"

#include "gl2Shaders.h"
static bool oglInitialized=false;

void oglInitialize(unsigned int sw,unsigned int sh)
{
    if (oglInitialized) return;
    ShaderEngine::Engine=new ogl2ShaderEngine();
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
