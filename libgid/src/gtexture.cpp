#include <gtexture.h>

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>
#endif

#ifdef QT_CORE_LIB
#include <QtGlobal>
#endif

#if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
    #include <OpenGLES/ES1/gl.h>
    #include <OpenGLES/ES1/glext.h>
    #define OPENGL_ES
#elif __ANDROID__
    #include <GLES/gl.h>
    #include <GLES/glext.h>
    #define OPENGL_ES
#else
    #include <GL/glew.h>
    #define OPENGL_DESKTOP
#endif

#ifdef OPENGL_ES
#define GL_FRAMEBUFFER GL_FRAMEBUFFER_OES
#define GL_COLOR_ATTACHMENT0 GL_COLOR_ATTACHMENT0_OES
#define GL_FRAMEBUFFER_BINDING GL_FRAMEBUFFER_BINDING_OES
#define glGenFramebuffers glGenFramebuffersOES
#define glDeleteFramebuffers glDeleteFramebuffersOES
#define glBindFramebuffer glBindFramebufferOES
#define glFramebufferTexture2D glFramebufferTexture2DOES
#endif


#include <assert.h>
#include <string.h>
#include <map>
#include <vector>
#include <set>
#include <string>
#include <stdlib.h>

#include <glog.h>

#include <snappy-c.h>

#include <algorithm>

namespace g_private {

struct CommonElement
{
    int refcount;
    int width;
    int height;
    int format;
    int type;
    int wrap;
    int filter;
    GLuint internalId;
};

struct TextureElement : public CommonElement
{
    size_t textureSize;
    std::vector<char> buffer;
    std::vector<char> signature;
    void *udata;
};

struct RenderTargetElement : public CommonElement
{
    GLuint framebuffer;
    size_t textureSize;
    std::vector<char> buffer;
    void *udata;
};

static int pixelSize(int format, int type)
{
    switch (type)
    {
    case GTEXTURE_UNSIGNED_SHORT_5_6_5:
    case GTEXTURE_UNSIGNED_SHORT_4_4_4_4:
    case GTEXTURE_UNSIGNED_SHORT_5_5_5_1:
        return 2;
    case GTEXTURE_UNSIGNED_BYTE:
        switch (format)
        {
        case GTEXTURE_ALPHA:
        case GTEXTURE_LUMINANCE:
            return 1;
        case GTEXTURE_LUMINANCE_ALPHA:
            return 2;
        case GTEXTURE_RGB:
            return 3;
        case GTEXTURE_RGBA:
            return 4;
        }
    }

    return 0;
}

class TextureManager
{
public:
    TextureManager()
    {
        nextid_ = 1;
        caching_ = false;
        textureMemory_ = 0;
        bufferMemory_ = 0;
        //glog_setLevel(GLOG_VERBOSE);
    }

    ~TextureManager()
    {
        while (!textureElements_.empty())
            deleteTexture(textureElements_.begin()->first);
        tick();
    }

    g_id create(int width, int height,
                int format, int type,
                int wrap, int filter,
                const void *pixels,
                const void *signature, size_t siglength)
    {
        TextureElement *element = new TextureElement;

        element->refcount = 1;
        element->width = width;
        element->height = height;
        element->format = format;
        element->type = type;
        element->wrap = wrap;
        element->filter = filter;
        element->udata = NULL;

        genAndUploadTexture(element, pixels);

        element->textureSize = width * height * pixelSize(format, type);

        textureMemory_ += element->textureSize;

        if (caching_)
        {
            size_t output_length = snappy_max_compressed_length(element->textureSize);
            element->buffer.resize(output_length);
            snappy_compress((const char*)pixels, element->textureSize, &element->buffer[0], &output_length);
            element->buffer.resize(output_length);
            bufferMemory_ += element->buffer.size();
        }

        if (siglength > 0)
        {
            element->signature.resize(sizeof(format) + sizeof(type) + sizeof(wrap) + sizeof(filter) + siglength);

            char* ptr = &element->signature[0];

            memcpy(ptr, signature, siglength);
            ptr += siglength;
            memcpy(ptr, &format, sizeof(format));
            ptr += sizeof(format);
            memcpy(ptr, &type, sizeof(type));
            ptr += sizeof(type);
            memcpy(ptr, &wrap, sizeof(wrap));
            ptr += sizeof(wrap);
            memcpy(ptr, &filter, sizeof(filter));
            ptr += sizeof(filter);

            assert(signatureMap_.find(element->signature) == signatureMap_.end());

            signatureMap_[element->signature] = element;
        }

        textureElements_[nextid_] = element;

        const char* name = !element->signature.empty() ? (char*)&element->signature[0] : "*unnamed*";
        glog_v("Creating texture %s. Total memory is %g KB.", name, (bufferMemory_ + textureMemory_) / 1024.0);

        return nextid_++;
    }

    g_bool deleteTexture(g_id id)
    {
        {
            std::map<g_id, TextureElement*>::iterator iter = textureElements_.find(id);

            if (iter != textureElements_.end())
            {
                TextureElement* element = iter->second;

                if (--element->refcount == 0)
                {
                    textureMemory_ -= element->textureSize;

                    bufferMemory_ -= element->buffer.size();

                    glDeleteTextures(1, &element->internalId);

                    signatureMap_.erase(element->signature);

                    const char* name = !element->signature.empty() ? (char*)&element->signature[0] : "*unnamed*";
                    glog_v("Deleting texture %s. Total memory is %g KB.", name, (bufferMemory_ + textureMemory_) / 1024.0);

                    delete element;

                    textureElements_.erase(iter);

                    return g_true;
                }

                const char* name = !element->signature.empty() ? (char*)&element->signature[0] : "*unnamed*";
                glog_v("Decreasing refcount of %s. New refcount is %d.", name, element->refcount);

                textureElements_.erase(iter);

                return g_false;
            }
        }

        {
            std::map<g_id, RenderTargetElement*>::iterator iter = renderTargetElements_.find(id);

            if (iter != renderTargetElements_.end())
            {
                RenderTargetElement *element = iter->second;

                textureMemory_ -= element->textureSize;

                glog_v("Deleting render target. Total memory is %g KB.", (bufferMemory_ + textureMemory_) / 1024.0);

                glDeleteFramebuffers(1, &element->framebuffer);

                glDeleteTextures(1, &element->internalId);

                delete element;

                renderTargetElements_.erase(iter);

                return g_true;
            }
        }

        return g_false;
    }

    void tick()
    {
    }

    GLuint getInternalId(g_id id)
    {
        {
            std::map<g_id, TextureElement*>::iterator iter = textureElements_.find(id);

            if (iter != textureElements_.end())
                return iter->second->internalId;
        }

        {
            std::map<g_id, RenderTargetElement*>::iterator iter = renderTargetElements_.find(id);

            if (iter != renderTargetElements_.end())
                return iter->second->internalId;
        }

        return 0;
    }

    void setUserData(g_id id, void *udata)
    {
        {
            std::map<g_id, TextureElement*>::iterator iter = textureElements_.find(id);

            if (iter != textureElements_.end())
                iter->second->udata = udata;
        }

        {
            std::map<g_id, RenderTargetElement*>::iterator iter = renderTargetElements_.find(id);

            if (iter != renderTargetElements_.end())
                iter->second->udata = udata;
        }

    }

    void *getUserData(g_id id)
    {
        {
            std::map<g_id, TextureElement*>::iterator iter = textureElements_.find(id);

            if (iter != textureElements_.end())
                return iter->second->udata;
        }

        {
            std::map<g_id, RenderTargetElement*>::iterator iter = renderTargetElements_.find(id);

            if (iter != renderTargetElements_.end())
                return iter->second->udata;
        }

        return NULL;
    }

    void setCachingEnabled(int caching)
    {
        caching_ = caching;
    }

    void reloadTextures()
    {
        std::set<TextureElement*> textureElements;
        std::map<g_id, TextureElement*>::iterator iter2, e2 = textureElements_.end();
        for (iter2 = textureElements_.begin(); iter2 != e2; ++iter2)
            textureElements.insert(iter2->second);

        std::set<TextureElement*>::iterator iter, e = textureElements.end();
        for (iter = textureElements.begin(); iter != e; ++iter)
        {
            TextureElement *element = *iter;

            size_t output_length;
            snappy_uncompressed_length(&element->buffer[0], element->buffer.size(), &output_length);
            char* output = (char*)malloc(output_length);
            snappy_uncompress(&element->buffer[0], element->buffer.size(), output, &output_length);
            genAndUploadTexture(element, output);
            free(output);
        }
    }

    void RestoreRenderTargets()
    {
        GLint oldFBO = 0;
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &oldFBO);

        std::map<g_id, RenderTargetElement*>::iterator iter, e = renderTargetElements_.end();
        for (iter = renderTargetElements_.begin(); iter != e; ++iter)
        {
            RenderTargetElement *element = iter->second;

            size_t output_length;
            snappy_uncompressed_length(&element->buffer[0], element->buffer.size(), &output_length);
            std::vector<char> uncompressed(output_length);
            snappy_uncompress(&element->buffer[0], element->buffer.size(), &uncompressed[0], &output_length);
            std::vector<char>().swap(element->buffer);
            genAndUploadTexture(element, &uncompressed[0]);

            glGenFramebuffers(1, &element->framebuffer);
            glBindFramebuffer(GL_FRAMEBUFFER, element->framebuffer);

            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, element->internalId, 0);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, oldFBO);
    }

    g_id reuse(int format, int type,
               int wrap, int filter,
               const void *signature, size_t siglength)
    {
        if (siglength == 0)
            return 0;

        std::vector<char> sig(sizeof(format) + sizeof(type) + sizeof(wrap) + sizeof(filter) + siglength);

        char* ptr = &sig[0];

        memcpy(ptr, signature, siglength);
        ptr += siglength;
        memcpy(ptr, &format, sizeof(format));
        ptr += sizeof(format);
        memcpy(ptr, &type, sizeof(type));
        ptr += sizeof(type);
        memcpy(ptr, &wrap, sizeof(wrap));
        ptr += sizeof(wrap);
        memcpy(ptr, &filter, sizeof(filter));
        ptr += sizeof(filter);

        std::map<std::vector<char>, TextureElement*>::iterator iter = signatureMap_.find(sig);

        if (iter == signatureMap_.end())
            return 0;

        TextureElement *element = iter->second;
        element->refcount++;
        textureElements_[nextid_] = element;

        const char* name = !element->signature.empty() ? (char*)&element->signature[0] : "*unnamed*";
        glog_v("Increasing refcount of %s. New refcount is %d.", name, element->refcount);

        return nextid_++;
    }

    size_t getMemoryUsage() const
    {
        return textureMemory_ + bufferMemory_;
    }


    g_id RenderTargetCreate(int width, int height,
                            int wrap, int filter)
    {
        int format = GTEXTURE_RGBA;
        int type = GTEXTURE_UNSIGNED_BYTE;

        RenderTargetElement *element = new RenderTargetElement;

        element->refcount = 1;
        element->width = width;
        element->height = height;
        element->format = format;
        element->type = type;
        element->wrap = wrap;
        element->filter = filter;
        element->udata = NULL;

        element->textureSize = width * height * pixelSize(format, type);

        void *pixels = malloc(element->textureSize);
        memset(pixels, 0, element->textureSize);
        genAndUploadTexture(element, pixels);
        free(pixels);

        textureMemory_ += element->textureSize;
        
        glog_v("Creating render target. Total memory is %g KB.", (bufferMemory_ + textureMemory_) / 1024.0);

        GLint oldFBO;
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &oldFBO);

        glGenFramebuffers(1, &element->framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, element->framebuffer);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, element->internalId, 0);

        glBindFramebuffer(GL_FRAMEBUFFER, oldFBO);

        renderTargetElements_[nextid_] = element;

        return nextid_++;
    }


    GLuint RenderTargetGetFBO(g_id renderTarget)
    {
        std::map<g_id, RenderTargetElement*>::iterator iter = renderTargetElements_.find(renderTarget);

        if (iter == renderTargetElements_.end())
            return 0;

        return iter->second->framebuffer;
    }

    void SaveRenderTargets()
    {
        GLint oldFBO = 0;
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &oldFBO);

        std::map<g_id, RenderTargetElement*>::iterator iter, e = renderTargetElements_.end();
        for (iter = renderTargetElements_.begin(); iter != e; ++iter)
        {
            RenderTargetElement *element = iter->second;
            glBindFramebuffer(GL_FRAMEBUFFER, element->framebuffer);
            glPixelStorei(GL_PACK_ALIGNMENT, 1);
            element->buffer.resize(element->width * element->height * 4);
            glReadPixels(0, 0, element->width, element->height, GL_RGBA, GL_UNSIGNED_BYTE, &element->buffer[0]);
            size_t output_length = snappy_max_compressed_length(element->buffer.size());
            std::vector<char> compressed(output_length);
            snappy_compress(&element->buffer[0], element->buffer.size(), &compressed[0], &output_length);
            compressed.resize(output_length);
            element->buffer = compressed;
            glPixelStorei(GL_PACK_ALIGNMENT, 4);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, oldFBO);
    }

    g_id TempTextureCreate(int width, int height)
    {
        std::map<g_id, TempTextureElement*>::iterator iter, e = tempTextureElements_.end();

        for (iter = tempTextureElements_.begin(); iter != e; ++iter)
            if (iter->second->width == width && iter->second->height == height)
                break;

        if (iter == tempTextureElements_.end())
        {
            TempTextureElement *element = new TempTextureElement;
            element->refcount = 1;
            element->width = width;
            element->height = height;

            GLint oldTex = 0;
            glGetIntegerv(GL_TEXTURE_BINDING_2D, &oldTex);

            glGenTextures(1, &element->name);
            glBindTexture(GL_TEXTURE_2D, element->name);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

            glBindTexture(GL_TEXTURE_2D, oldTex);

            tempTextureElements_[nextid_] = element;
        }
        else
        {
            TempTextureElement *element = iter->second;
            element->refcount++;
            tempTextureElements_[nextid_] = element;
        }

        return nextid_++;
    }

    void TempTextureDelete(g_id id)
    {
        std::map<g_id, TempTextureElement*>::iterator iter = tempTextureElements_.find(id);
        if (iter == tempTextureElements_.end())
            return;

        TempTextureElement *element = iter->second;

        if (--element->refcount == 0)
        {
            glDeleteTextures(1, &element->name);
            delete element;
        }

        tempTextureElements_.erase(iter);
    }

    GLuint TempTextureGetName(g_id id)
    {
        std::map<g_id, TempTextureElement*>::iterator iter = tempTextureElements_.find(id);
        if (iter == tempTextureElements_.end())
            return 0;

        return iter->second->name;
    }

    struct TempTextureElement
    {
        int refcount;
        int width;
        int height;
        GLuint name;
    };

    void RestoreTempTextures()
    {
        GLint oldTex = 0;
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &oldTex);

        std::set<TempTextureElement*> tempTextureElements;
        std::map<g_id, TempTextureElement*>::iterator iter2, e2 = tempTextureElements_.end();
        for (iter2 = tempTextureElements_.begin(); iter2 != e2; ++iter2)
            tempTextureElements.insert(iter2->second);

        std::set<TempTextureElement*>::iterator iter, e = tempTextureElements.end();
        for (iter = tempTextureElements.begin(); iter != e; ++iter)
        {
            TempTextureElement *element = *iter;

            glGenTextures(1, &element->name);
            glBindTexture(GL_TEXTURE_2D, element->name);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, element->width, element->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        }

        glBindTexture(GL_TEXTURE_2D, oldTex);
    }

private:
    std::map<g_id, TempTextureElement*> tempTextureElements_;

private:
    void genAndUploadTexture(CommonElement *element, const void *pixels)
    {
        GLenum format;
        switch (element->format)
        {
        case GTEXTURE_ALPHA:
            format = GL_ALPHA;
            break;
        case GTEXTURE_RGB:
            format = GL_RGB;
            break;
        case GTEXTURE_RGBA:
            format = GL_RGBA;
            break;
        case GTEXTURE_LUMINANCE:
            format = GL_LUMINANCE;
            break;
        case GTEXTURE_LUMINANCE_ALPHA:
            format = GL_LUMINANCE_ALPHA;
            break;
        }

        GLenum type;
        switch (element->type)
        {
        case GTEXTURE_UNSIGNED_BYTE:
            type = GL_UNSIGNED_BYTE;
            break;
        case GTEXTURE_UNSIGNED_SHORT_5_6_5:
            type = GL_UNSIGNED_SHORT_5_6_5;
            break;
        case GTEXTURE_UNSIGNED_SHORT_4_4_4_4:
            type = GL_UNSIGNED_SHORT_4_4_4_4;
            break;
        case GTEXTURE_UNSIGNED_SHORT_5_5_5_1:
            type = GL_UNSIGNED_SHORT_5_5_5_1;
            break;
        }

        GLint oldTex = 0;
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &oldTex);

        glGenTextures(1, &element->internalId);
        glBindTexture(GL_TEXTURE_2D, element->internalId);
        switch (element->wrap)
        {
        case GTEXTURE_CLAMP:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            break;
        case GTEXTURE_REPEAT:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            break;
        }
        switch (element->filter)
        {
        case GTEXTURE_NEAREST:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            break;
        case GTEXTURE_LINEAR:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            break;
        }

        glTexImage2D(GL_TEXTURE_2D, 0, format, element->width, element->height, 0, format, type, pixels);

        glBindTexture(GL_TEXTURE_2D, oldTex);
    }

private:
    bool caching_;
    g_id nextid_;
    std::map<g_id, TextureElement*> textureElements_;
    std::map<std::vector<char>, TextureElement*> signatureMap_;
    std::map<g_id, RenderTargetElement*> renderTargetElements_;
    size_t textureMemory_;
    size_t bufferMemory_;
};

}

using namespace g_private;

static TextureManager *s_manager = NULL;

extern "C" {

void gtexture_init()
{
    s_manager = new TextureManager;
}

void gtexture_cleanup()
{
    delete s_manager;
    s_manager = NULL;
}

g_id gtexture_create(int width, int height,
                     int format, int type,
                     int wrap, int filter,
                     const void *pixels,
                     const void *signature, size_t siglength)
{
    return s_manager->create(width, height,
                             format, type,
                             wrap, filter,
                             pixels,
                             signature, siglength);
}

g_bool gtexture_delete(g_id id)
{
    return s_manager->deleteTexture(id);
}

unsigned int gtexture_getInternalId(g_id id)
{
    return s_manager->getInternalId(id);
}

void gtexture_setUserData(g_id id, void *udata)
{
    s_manager->setUserData(id, udata);
}

void *gtexture_getUserData(g_id id)
{
    return s_manager->getUserData(id);
}

void gtexture_tick()
{
    s_manager->tick();
}

void gtexture_setCachingEnabled(int caching)
{
    s_manager->setCachingEnabled(caching);
}

void gtexture_reloadTextures()
{
    s_manager->reloadTextures();
}

g_id gtexture_reuse(int format, int type,
                    int wrap, int filter,
                    const void *signature, size_t siglength)
{
    return s_manager->reuse(format, type,
                            wrap, filter,
                            signature, siglength);
}

G_API size_t gtexture_getMemoryUsage()
{
    return s_manager->getMemoryUsage();
}

g_id gtexture_RenderTargetCreate(int width, int height,
                                 int wrap, int filter)
{
    return s_manager->RenderTargetCreate(width, height, wrap, filter);
}

unsigned int gtexture_RenderTargetGetFBO(g_id renderTarget)
{
    return s_manager->RenderTargetGetFBO(renderTarget);
}

void gtexture_SaveRenderTargets()
{
    s_manager->SaveRenderTargets();
}

void gtexture_RestoreRenderTargets()
{
    s_manager->RestoreRenderTargets();
}

g_id gtexture_TempTextureCreate(int width, int height)
{
    return s_manager->TempTextureCreate(width, height);
}

void gtexture_TempTextureDelete(g_id id)
{
    s_manager->TempTextureDelete(id);
}

unsigned int gtexture_TempTextureGetName(g_id id)
{
    return s_manager->TempTextureGetName(id);
}

void gtexture_RestoreTempTextures()
{
    s_manager->RestoreTempTextures();
}

}
