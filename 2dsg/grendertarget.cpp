#include "grendertarget.h"
#include "application.h"
#include "sprite.h"
#include "glcommon.h"

int GRenderTarget::qualcommFix_ = -1;

GRenderTarget::GRenderTarget(Application *application, int width, int height, Filter filter) :
    TextureBase(application)
{
    TextureParameters parameters;
    parameters.filter = filter;
    data = application->getTextureManager()->createRenderTarget(width, height, parameters);

    sizescalex = 1;
    sizescaley = 1;
    uvscalex = (float)data->width / (float)data->baseWidth;
    uvscaley = (float)data->height / (float)data->baseHeight;

    if (qualcommFix_ == -1)
    {
        const char *extensions = (const char *)glGetString(GL_EXTENSIONS);
        qualcommFix_ = (extensions && strstr(extensions, "GL_QCOM"));
    }

    if (qualcommFix_)
        tempTexture_ = gtexture_TempTextureCreate(data->exwidth, data->exheight);
    else
        tempTexture_ = 0;
}

GRenderTarget::~GRenderTarget()
{
    if (tempTexture_)
        gtexture_TempTextureDelete(tempTexture_);
}

void GRenderTarget::clear(unsigned int color, float a)
{
    GLint oldFBO = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &oldFBO);

    glBindFramebuffer(GL_FRAMEBUFFER, gtexture_RenderTargetGetFBO(data->gid));

    glViewport(0, 0, data->width, data->height);

    float r = ((color >> 16) & 0xff) / 255.f;
    float g = ((color >> 8) & 0xff) / 255.f;
    float b = (color & 0xff) / 255.f;

    glClearColor(r * a, g * a, b * a, a);
    glClear(GL_COLOR_BUFFER_BIT);

    glBindFramebuffer(GL_FRAMEBUFFER, oldFBO);
}

void GRenderTarget::draw(const Sprite *sprite)
{
    GLint oldFBO = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &oldFBO);

    glBindFramebuffer(GL_FRAMEBUFFER, gtexture_RenderTargetGetFBO(data->gid));

    if (qualcommFix_)
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gtexture_TempTextureGetName(tempTexture_), 0);
        glClear(GL_COLOR_BUFFER_BIT);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gtexture_getInternalId(data->gid), 0);
    }

    glViewport(0, 0, data->width, data->height);

    //glOrthof(0, data->baseWidth, 0, data->baseHeight, -1, 1);

    CurrentTransform currentTransform;
    ((Sprite*)sprite)->draw(currentTransform, 0, 0, data->width, data->height);

    glBindFramebuffer(GL_FRAMEBUFFER, oldFBO);
}
