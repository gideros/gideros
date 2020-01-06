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
static ShaderEngine *engine =NULL;
static ScreenManager *screenManager = NULL;
static SpriteProxyFactory *spritefactory=NULL;

struct CommonElement
{
    int refcount;
    int width;
    int height;
    int format;
    int type;
    int wrap;
    int filter;
    bool renderTarget;
    ShaderTexture *_texture;
};

struct TextureElement : public CommonElement
{
    size_t textureSize;
    char *buffer;
    size_t bufferSize;
    std::vector<char> signature;
    void *udata;
};

struct RenderTargetElement : public CommonElement
{
    ShaderBuffer *_framebuffer;
    size_t textureSize;
    bool depth;
    char *buffer;
    size_t bufferSize;
    void *udata;
};

static int pixelSize(int format, int type)
{
    switch (type)
    {
    case GTEXTURE_UNSIGNED_SHORT_5_6_5:
    case GTEXTURE_UNSIGNED_SHORT_4_4_4_4:
    case GTEXTURE_UNSIGNED_SHORT_5_5_5_1:
    case GTEXTURE_UNSIGNED_SHORT:
        return 2;
    case GTEXTURE_FLOAT:
    case GTEXTURE_UNSIGNED_INT:
        return 4;
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
        element->renderTarget = false;
        element->buffer=NULL;
        element->bufferSize=0;

        genAndUploadTexture(element, pixels);

        element->textureSize = width * height * pixelSize(format, type);

        textureMemory_ += element->textureSize;

        if (caching_)
        {
            size_t output_length = snappy_max_compressed_length(element->textureSize);
            char *temp=new char[output_length];
            snappy_compress((const char*)pixels, element->textureSize, temp, &output_length);
            element->buffer=new char[output_length];
            memcpy(element->buffer,temp,output_length);
            delete[] temp;
            element->bufferSize=output_length;
            bufferMemory_ += element->bufferSize;
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

    void update(g_id gid,
    		    int width, int height,
                int format, int type,
                int wrap, int filter,
                const void *pixels)
    {
    	std::map<g_id, TextureElement*>::iterator iter = textureElements_.find(gid);

    	if (iter != textureElements_.end())
    	{
    	     TextureElement* element = iter->second;

    	        ShaderTexture::Format format;
    	        switch (element->format)
    	        {
    	        case GTEXTURE_ALPHA:
    	            format = ShaderTexture::FMT_ALPHA;
    	            break;
    	        case GTEXTURE_RGB:
    	            format = ShaderTexture::FMT_RGB;
    	            break;
    	        case GTEXTURE_RGBA:
    	            format = ShaderTexture::FMT_RGBA;
    	            break;
    	        case GTEXTURE_LUMINANCE:
    	            format = ShaderTexture::FMT_Y;
    	            break;
    	        case GTEXTURE_LUMINANCE_ALPHA:
    	            format = ShaderTexture::FMT_YA;
    	            break;
    	        }

    	        ShaderTexture::Packing type;
    	        switch (element->type)
    	        {
    	        case GTEXTURE_UNSIGNED_BYTE:
    	            type = ShaderTexture::PK_UBYTE;
    	            break;
    	        case GTEXTURE_UNSIGNED_SHORT_5_6_5:
    	            type = ShaderTexture::PK_USHORT_565;
    	            break;
    	        case GTEXTURE_UNSIGNED_SHORT_4_4_4_4:
    	            type = ShaderTexture::PK_USHORT_4444;
    	            break;
    	        case GTEXTURE_UNSIGNED_SHORT_5_5_5_1:
    	            type = ShaderTexture::PK_USHORT_5551;
    	            break;
    	        }
    	        ShaderTexture::Wrap wrap=ShaderTexture::WRAP_CLAMP;
    	        if (element->wrap==GTEXTURE_REPEAT)
    	        	wrap=ShaderTexture::WRAP_REPEAT;
    	        ShaderTexture::Filtering filtering=ShaderTexture::FILT_LINEAR;
    	        if (element->filter==GTEXTURE_NEAREST)
    	        	filtering=ShaderTexture::FILT_NEAREST;
    	     element->_texture->updateData(format,type,element->width, element->height,pixels,wrap,filtering);

    	     textureMemory_ -= element->textureSize;
    	     element->textureSize = width * height * pixelSize(format, type);
    	     textureMemory_ += element->textureSize;

    	     if (caching_)
    	     {
    	    	 bufferMemory_-=element->bufferSize;
    	    	 size_t output_length = snappy_max_compressed_length(element->textureSize);
    	         char *temp=new char[output_length];
    	         snappy_compress((const char*)pixels, element->textureSize, temp, &output_length);
    	         if (element->buffer) delete[] element->buffer;
    	         element->buffer=new char[output_length];
    	         memcpy(element->buffer,temp,output_length);
   	             delete[] temp;
   	             element->bufferSize=output_length;
    	    	 bufferMemory_ += element->bufferSize;
    	     }
    	}
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

                    bufferMemory_ -= element->bufferSize;
                    if (element->buffer) delete[] element->buffer;

                    delete element->_texture;

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

                if (element->buffer) delete[] element->buffer;

                delete element->_framebuffer;

                delete element->_texture;

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

    ShaderTexture *getInternalTexture(g_id id)
    {
        {
            std::map<g_id, TextureElement*>::iterator iter = textureElements_.find(id);

            if (iter != textureElements_.end())
                return iter->second->_texture;
        }

        {
            std::map<g_id, RenderTargetElement*>::iterator iter = renderTargetElements_.find(id);

            if (iter != renderTargetElements_.end())
                return iter->second->_texture;
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
            snappy_uncompressed_length(element->buffer, element->bufferSize, &output_length);
            char* output = (char*)malloc(output_length);
            snappy_uncompress(element->buffer, element->bufferSize, output, &output_length);
            genAndUploadTexture(element, output);
            free(output);
        }
    }

    void RestoreRenderTargets()
    {
        if (renderTargetElements_.empty())
            return;

        std::map<g_id, RenderTargetElement*>::iterator iter, e = renderTargetElements_.end();
        for (iter = renderTargetElements_.begin(); iter != e; ++iter)
        {
            RenderTargetElement *element = iter->second;

            size_t output_length;
            snappy_uncompressed_length(element->buffer, element->bufferSize, &output_length);
            char* output = (char*)malloc(output_length);
            snappy_uncompress(element->buffer, element->bufferSize, output, &output_length);
            delete[] element->buffer;
            element->buffer=NULL;
            genAndUploadTexture(element, output);
            free(output);

            element->_framebuffer=engine->createRenderTarget(element->_texture,element->depth);
        }

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
                            int wrap, int filter, bool depth)
    {
        int format = depth?GTEXTURE_DEPTH:GTEXTURE_RGBA;
        ShaderTexture::Packing pk=engine->getPreferredPackingForTextureFormat(depth?ShaderTexture::FMT_DEPTH:ShaderTexture::FMT_RGBA);
        int type=GTEXTURE_UNSIGNED_BYTE;
        switch (pk) {
        case ShaderTexture::PK_FLOAT: type=GTEXTURE_FLOAT; break;
        case ShaderTexture::PK_USHORT: type=GTEXTURE_UNSIGNED_SHORT; break;
        case ShaderTexture::PK_UINT: type=GTEXTURE_UNSIGNED_INT; break;
        case ShaderTexture::PK_USHORT_4444: type=GTEXTURE_UNSIGNED_SHORT_4_4_4_4; break;
        case ShaderTexture::PK_USHORT_565: type=GTEXTURE_UNSIGNED_SHORT_5_6_5; break;
        case ShaderTexture::PK_USHORT_5551: type=GTEXTURE_UNSIGNED_SHORT_5_5_5_1; break;
        default: type=GTEXTURE_UNSIGNED_BYTE;
        }

        RenderTargetElement *element = new RenderTargetElement;

		//Ensure requested size is never negative or 0
		if (width <= 0) width = 1;
		if (height <= 0) height = 1;


        element->refcount = 1;
        element->width = width;
        element->height = height;
        element->format = format;
        element->type = type;
        element->wrap = wrap;
        element->filter = filter;
        element->udata = NULL;
        element->renderTarget = true;
        element->depth=depth;
        element->buffer=NULL;
        element->bufferSize=0;

        element->textureSize = width * height * pixelSize(format, type);

        void *pixels = malloc(element->textureSize);
        memset(pixels, 0, element->textureSize);
        genAndUploadTexture(element, pixels);
        free(pixels);

        textureMemory_ += element->textureSize;
        
        glog_v("Creating render target. Total memory is %g KB.", (bufferMemory_ + textureMemory_) / 1024.0);

        element->_framebuffer=engine->createRenderTarget(element->_texture,element->depth);

        renderTargetElements_[nextid_] = element;

        return nextid_++;
    }


    ShaderBuffer *RenderTargetGetFBO(g_id renderTarget)
    {
        std::map<g_id, RenderTargetElement*>::iterator iter = renderTargetElements_.find(renderTarget);

        if (iter == renderTargetElements_.end())
            return 0;

        return iter->second->_framebuffer;
    }

    void SaveRenderTargets()
    {
        if (renderTargetElements_.empty())
            return;

        std::map<g_id, RenderTargetElement*>::iterator iter, e = renderTargetElements_.end();
        for (iter = renderTargetElements_.begin(); iter != e; ++iter)
        {
            RenderTargetElement *element = iter->second;
            char *pixels=(char *)malloc(element->width * element->height * 4);
            element->_framebuffer->readPixels(0, 0, element->width, element->height, ShaderTexture::FMT_RGBA, ShaderTexture::PK_UBYTE, pixels);
            size_t output_length = snappy_max_compressed_length(element->width * element->height * 4);
            char *temp=(char *)malloc(output_length);
            snappy_compress(pixels,element->width * element->height * 4, temp, &output_length);
            free(pixels);
            element->buffer = new char[output_length];
            memcpy(element->buffer,temp,output_length);
            free(temp);
        }

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


            element->name=engine->createTexture(ShaderTexture::FMT_RGBA,ShaderTexture::PK_UBYTE,
            		element->width,element->height,NULL,
            		ShaderTexture::WRAP_CLAMP,ShaderTexture::FILT_NEAREST);

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
            delete element->name;
            delete element;
        }

        tempTextureElements_.erase(iter);
    }

    ShaderTexture *TempTextureGetName(g_id id)
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
        ShaderTexture *name;
    };

    void RestoreTempTextures()
    {
        std::set<TempTextureElement*> tempTextureElements;
        std::map<g_id, TempTextureElement*>::iterator iter2, e2 = tempTextureElements_.end();
        for (iter2 = tempTextureElements_.begin(); iter2 != e2; ++iter2)
            tempTextureElements.insert(iter2->second);

        std::set<TempTextureElement*>::iterator iter, e = tempTextureElements.end();
        for (iter = tempTextureElements.begin(); iter != e; ++iter)
        {
            TempTextureElement *element = *iter;

            element->name=engine->createTexture(ShaderTexture::FMT_RGBA,ShaderTexture::PK_UBYTE,
            		element->width,element->height,NULL,
            		ShaderTexture::WRAP_CLAMP,ShaderTexture::FILT_NEAREST);
        }
    }

private:
    std::map<g_id, TempTextureElement*> tempTextureElements_;

private:
    void genAndUploadTexture(CommonElement *element, const void *pixels)
    {
        ShaderTexture::Format format;
        switch (element->format)
        {
        case GTEXTURE_ALPHA:
            format = ShaderTexture::FMT_ALPHA;
            break;
        case GTEXTURE_RGB:
            format = ShaderTexture::FMT_RGB;
            break;
        case GTEXTURE_RGBA:
            format = ShaderTexture::FMT_RGBA;
            break;
        case GTEXTURE_LUMINANCE:
            format = ShaderTexture::FMT_Y;
            break;
        case GTEXTURE_LUMINANCE_ALPHA:
            format = ShaderTexture::FMT_YA;
            break;
        case GTEXTURE_DEPTH:
            format = ShaderTexture::FMT_DEPTH;
            break;
        }

        ShaderTexture::Packing type;
        switch (element->type)
        {
        case GTEXTURE_UNSIGNED_BYTE:
            type = ShaderTexture::PK_UBYTE;
            break;
        case GTEXTURE_UNSIGNED_SHORT_5_6_5:
            type = ShaderTexture::PK_USHORT_565;
            break;
        case GTEXTURE_UNSIGNED_SHORT_4_4_4_4:
            type = ShaderTexture::PK_USHORT_4444;
            break;
        case GTEXTURE_UNSIGNED_SHORT_5_5_5_1:
            type = ShaderTexture::PK_USHORT_5551;
            break;
        case GTEXTURE_FLOAT:
            type = ShaderTexture::PK_FLOAT;
            break;
        case GTEXTURE_UNSIGNED_SHORT:
            type = ShaderTexture::PK_USHORT;
            break;
        case GTEXTURE_UNSIGNED_INT:
            type = ShaderTexture::PK_UINT;
            break;
        }
        ShaderTexture::Wrap wrap=ShaderTexture::WRAP_CLAMP;
        if (element->wrap==GTEXTURE_REPEAT)
        	wrap=ShaderTexture::WRAP_REPEAT;
        ShaderTexture::Filtering filtering=ShaderTexture::FILT_LINEAR;
        if (element->filter==GTEXTURE_NEAREST)
        	filtering=ShaderTexture::FILT_NEAREST;
        element->_texture=engine->createTexture(format,type,element->width, element->height,pixels,wrap,filtering,element->renderTarget);
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

void gtexture_set_engine(ShaderEngine *e)
{
	engine=e;
}

ShaderEngine *gtexture_get_engine()
{
	return engine;
}

void gtexture_set_spritefactory(SpriteProxyFactory *e)
{
	spritefactory=e;
}

SpriteProxyFactory *gtexture_get_spritefactory()
{
	return spritefactory;
}

void gtexture_set_screenmanager(ScreenManager *e)
{
	screenManager=e;
}

ScreenManager *gtexture_get_screenmanager()
{
	return screenManager;
}

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

void gtexture_update(g_id gid,
					 int width, int height,
                     int format, int type,
                     int wrap, int filter,
                     const void *pixels)
{
    return s_manager->update(gid, width, height,
                             format, type,
                             wrap, filter,
                             pixels);
}

g_bool gtexture_delete(g_id id)
{
    return s_manager->deleteTexture(id);
}

ShaderTexture *gtexture_getInternalTexture(g_id id)
{
    return s_manager->getInternalTexture(id);
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
                                 int wrap, int filter,bool depth)
{
    return s_manager->RenderTargetCreate(width, height, wrap, filter, depth);
}

ShaderBuffer *gtexture_RenderTargetGetFBO(g_id renderTarget)
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

ShaderTexture *gtexture_TempTextureGetName(g_id id)
{
    return s_manager->TempTextureGetName(id);
}

void gtexture_RestoreTempTextures()
{
    s_manager->RestoreTempTextures();
}

ShaderBuffer *gtexture_BindRenderTarget(ShaderBuffer *fbo)
{
	return engine->setFramebuffer(fbo);
}

static void *global_hooks[GID_GLOBALHOOK_MAX];
void g_setGlobalHook(unsigned int hookn,void *hook) {
	if (hookn>=GID_GLOBALHOOK_MAX) return;
	global_hooks[hookn]=hook;
}

void *g_getGlobalHook(unsigned int hookn) {
	if (hookn>=GID_GLOBALHOOK_MAX) return NULL;
	return global_hooks[hookn];
}

}
