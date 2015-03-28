#ifdef Q_OS_WIN
#define HAVE_BOOLEAN
#endif
#define cimg_display 0
#define cimg_use_jpeg
#define cimg_use_png

#include "media.h"
#include "CImg.h"
#include "gpath.h"
#include "gstdio.h"

// some Lua helper functions
#ifndef abs_index
#define abs_index(L, i) ((i) > 0 || (i) <= LUA_REGISTRYINDEX ? (i) : lua_gettop(L) + (i) + 1)
#endif


static lua_State *L = NULL;

static bool file_exists(const char *filename)
{
  return ( access( filename, F_OK ) != -1 );
}

static std::string copyFile(std::string path){
    //check extenstion if should be copied
    const char *ext = strrchr(path.c_str(), '.');
    if (ext)
        ext++;
    if (!strcasecmp(ext, "jpeg") ||
        !strcasecmp(ext, "jpg") ||
        !strcasecmp(ext, "png") ||
        !strcasecmp(ext, "wav") ||
        !strcasecmp(ext, "lua"))
    {
        //get file name
        const char *filename = strrchr(path.c_str(), '/');
        if(filename == NULL){
            filename = strrchr(path.c_str(), '\\');
            if(filename == NULL){
                filename = strrchr(path.c_str(), '|');
                if(filename == NULL){
                    filename = path.c_str();
                }
                else
                    filename++;
            }
            else
                filename++;
        }
        else
            filename++;
        std::string copyPath = std::string("|D|") + std::string(filename);

        //check maybe file exists already exists
        if(!file_exists(g_pathForFile(copyPath.c_str())))
        {

            G_FILE *filer = g_fopen(path.c_str(), "rb");
            G_FILE *filew = g_fopen(copyPath.c_str(),"wb");

            char buffer[100];
            int numr,numw;
            while(g_feof(filer)==0){
                if((numr=g_fread(buffer,1,100,filer))!=100){
                    if(g_ferror(filer)!=0){
                        g_fprintf(g_stderr,"read file error.\n");
                        exit(1);
                    }
                    else if(g_feof(filer)!=0);
                }
                if((numw=g_fwrite(buffer,1,numr,filew))!=numr){
                    g_fprintf(g_stderr,"write file error.\n");
                    exit(1);
                }
            }
            g_fclose(filer);
            g_fclose(filew);
        }
        return copyPath;
    }
    return path;
}

static int lua_print(lua_State* L, const char* str)
{
    glog_e(str);
    lua_getglobal(L, "_G");
    lua_getfield(L, -1, "print");
    lua_pushstring(L, str);
    lua_call(L, 1, 0);
    lua_pop(L, 1);
    return 0;
}

static void luaL_newweaktable(lua_State *L, const char *mode)
{
    lua_newtable(L);			// create table for instance list
    lua_pushstring(L, mode);
    lua_setfield(L, -2, "__mode");	  // set as weak-value table
    lua_pushvalue(L, -1);             // duplicate table
    lua_setmetatable(L, -2);          // set itself as metatable
}

static void luaL_rawgetptr(lua_State *L, int idx, void *ptr)
{
    idx = abs_index(L, idx);
    lua_pushlightuserdata(L, ptr);
    lua_rawget(L, idx);
}

static void luaL_rawsetptr(lua_State *L, int idx, void *ptr)
{
    idx = abs_index(L, idx);
    lua_pushlightuserdata(L, ptr);
    lua_insert(L, -2);
    lua_rawset(L, idx);
}

static const char* MEDIA_RECEIVE = "mediaReceive";
static const char* MEDIA_CANCEL = "mediaCancele";
static const char* VIDEO_COMPLETE = "videoComplete";

static char keyWeak = ' ';

class GMediaManager : public GEventDispatcherProxy
{
public:
    GMediaManager()
    {
        gmedia_init();
        gmedia_addCallback(callback_s, this);
    }

    ~GMediaManager()
    {
        gmedia_removeCallback(callback_s, this);
        gmedia_cleanup();
    }


    int isCameraAvailable()
    {
        return gmedia_isCameraAvailable();
    }

    void takePicture()
    {
        if(isCameraAvailable())
            gmedia_takePicture();
    }

    void takeScreenshot()
    {
        gmedia_takeScreenshot();
    }

    void getPicture()
    {
        gmedia_getPicture();
    }

    void savePicture(const char* path)
    {
        gmedia_savePicture(path);
    }

    void playVideo(const char* path, bool force)
    {
        gmedia_playVideo(path, force);
    }

private:
    static void callback_s(int type, void *event, void *udata)
    {
        static_cast<GMediaManager*>(udata)->callback(type, event);
    }

    void callback(int type, void *event)
    {
        dispatchEvent(type, event);
    }

    void dispatchEvent(int type, void *event)
    {
        luaL_rawgetptr(L, LUA_REGISTRYINDEX, &keyWeak);
        luaL_rawgetptr(L, -1, this);
        if (lua_isnil(L, -1))
        {
            lua_pop(L, 2);
            return;
        }

        lua_getfield(L, -1, "dispatchEvent");

        lua_pushvalue(L, -2);

        lua_getglobal(L, "Event");
        lua_getfield(L, -1, "new");
        lua_remove(L, -2);

        switch (type)
        {
            case GMEDIA_RECEIVED_EVENT:
                lua_pushstring(L, MEDIA_RECEIVE);
                break;
            case GMEDIA_CANCELED_EVENT:
                lua_pushstring(L, MEDIA_CANCEL);
                break;
            case GMEDIA_COMPLETED_EVENT:
                lua_pushstring(L, VIDEO_COMPLETE);
                break;
        }

        lua_call(L, 1, 1);

        if (type == GMEDIA_RECEIVED_EVENT)
        {
            gmedia_ReceivedEvent *event2 = (gmedia_ReceivedEvent*)event;

            lua_pushstring(L, event2->path);
            lua_setfield(L, -2, "path");
        }

        lua_call(L, 2, 0);

        lua_pop(L, 2);
    }
};

class GMedia : public GProxy
{
public:
    GMedia(const char* path)
    {
        path_ = strdup(path);
        try
        {
            image = cimg_library::CImg<>(path_);
            if(image.spectrum() < 4)
                image.resize(-100,-100,1,4,0,0).get_shared_channel(3).fill(255);
        }
        catch(cimg_library::CImgException){}
    }

    GMedia(int width, int height)
    {
        time_t rawtime;
        struct tm * timeinfo;
        char buffer[80];

        time (&rawtime);
        timeinfo = localtime(&rawtime);

        strftime(buffer,80,"%Y%m%d_%I%M%S",timeinfo);
        std::string str(buffer);
        std::string path = std::string("|D|") + str + "_gideros.png";
        path_ = strdup(g_pathForFile(path.c_str()));
        image = cimg_library::CImg<>(width, height, 1, 4, 0);
    }

    ~GMedia()
    {

    }

    const char* getPath(){
        return path_;
    }

    int getWidth(){
        if(image != NULL)
            return image.width();
        return 0;
    }

    int getHeight(){
        if(image != NULL)
            return image.height();
        return 0;
    }

    int getPixel(int x, int y, int channel){
        if(image != NULL)
        {
            x -= 1;
            y -= 1;
            x = (x < 0) ? 0 : x;
            x = (x >= image.width()) ? image.width()-1 : x;
            y = (y < 0) ? 0 : y;
            y = (y >= image.height()) ? image.height()-1 : y;
            return (int)image(x,y,0,channel);
        }
        return 0;
    }

    int getR(int x, int y){
        return getPixel(x, y, 0);
    }

    int getG(int x, int y){
        return getPixel(x, y, 1);
    }

    int getB(int x, int y){
        return getPixel(x, y, 2);
    }

    float getA(int x, int y){
        return getPixel(x, y, 3)/255.0;
    }

    void setPixel(int x, int y, int r, int g, int b, float a, bool blend){
        if(image != NULL)
        {
            x -= 1;
            y -= 1;
            x = (x < 0) ? 0 : x;
            x = (x >= image.width()) ? image.width()-1 : x;
            y = (y < 0) ? 0 : y;
            y = (y >= image.height()) ? image.height()-1 : y;

            if(!blend)
            {
                image(x,y,0,0) = r;
                image(x,y,0,1) = g;
                image(x,y,0,2) = b;
                image(x,y,0,3) = a*255;
            }
            else
            {
                const unsigned char color[] = { static_cast<unsigned char>(r),static_cast<unsigned char>(g),static_cast<unsigned char>(b) };
                image(x,y,0,3) = a;
                image.draw_point(x,y,color, a);
            }
        }
    }

    void resizeWidth(int width, bool fixed)
    {
        if(image != NULL)
        {
            int height;
            if(fixed)
                height = image.height()*width/image.width();
            else
                height = image.height();
            image.resize(width,height,-100,-100,5);
        }
    }

    void resizeHeight(int height, bool fixed)
    {
        if(image != NULL)
        {
            int width;
            if(fixed)
                width = image.width()*height/image.height();
            else
                width = image.width();
            image.resize(width, height, -100,-100,5);
        }
    }

    void resize(int width, int height, bool fixed, bool crop)
    {
        if(image != NULL)
        {
            if(fixed)
            {
                int dstWidth = width;
                int dstHeight = height;
                double koefW = ((double)width)/image.width();
                double koefH = ((double)height)/image.height();

                if(!crop)//letterbox
                {
                    dstHeight = (int)(((double)image.height())*koefW);
                    if(dstHeight>height)
                    {
                        dstHeight = height;
                        dstWidth = (int)(((double)image.width())*koefH);
                    }
                    image.resize(dstWidth, dstHeight, -100,-100,5);
                }
                else//covers width-height
                {
                    int x = 0;
                    int y = 0;
                    int w = dstWidth;
                    int h = (int)(((double)image.height())*koefW);
                    if(w<dstWidth || h<dstHeight)
                    {
                        h = dstHeight;
                        w = (int)(((double)image.width())*koefH);
                    }

                    x = fabs(dstWidth-w)/2;
                    y = fabs(dstHeight-h)/2;
                    image.resize(w, h, -100,-100,5);

                    image.get_crop(x, y, x+dstWidth-1, y+dstHeight-1);
                }
            }
            else
            {
                image.resize(width, height, -100,-100,5);
            }
        }
    }

    void crop(int x, int y, int width, int height)
    {
        if(image != NULL)
            image.get_crop(x, y, x+width-1, y+height-1);
    }

    void copy(const char* dest)
    {
        if(image != NULL)
        {
            const char *ext = strrchr(dest, '.');
            if (ext)
                ext++;
            if (!strcasecmp(ext, "jpeg") ||
                !strcasecmp(ext, "jpg"))
            {
                image.get_resize(-100,-100,1,3,0,0).save(dest);
            }
            else
                image.save(dest);
        }
    }

    void save(){
        if(image != NULL)
        {
            const char *ext = strrchr(path_, '.');
            if (ext)
                ext++;
            if (!strcasecmp(ext, "jpeg") ||
                !strcasecmp(ext, "jpg"))
            {
                image.get_resize(-100,-100,1,3,0,0).save(path_);
            }
            else
                image.save(path_);
        }
    }

    void setRotation(float angle){
        angle_ += angle;
        if(image != NULL)
            image.rotate(angle);
    }

    float getRotation(){
        return angle_;
    }

    void autocrop(int hexValue){
        if(image != NULL)
        {
            int r = ((hexValue >> 16) & 0xFF);
            int g = ((hexValue >> 8) & 0xFF);
            int b = ((hexValue) & 0xFF);
            const unsigned char color[] = { static_cast<unsigned char>(r),static_cast<unsigned char>(g),static_cast<unsigned char>(b) };
            image.autocrop(color);
        }
    }

    void autocrop(){
        if(image != NULL)
            image.autocrop();
    }

    void flip(const char *axis){
        if(image != NULL)
            image.mirror(axis);
    }

    void drawImage(int x, int y, GMedia *media, float alpha){
        drawImage(x, y, media->image, alpha);
    }

    void drawImage(int x, int y, const char* path, float alpha){
        cimg_library::CImg<unsigned char> image2 = cimg_library::CImg<>(path);
        drawImage(x, y, image2, alpha);
    }

    void drawImage(int x, int y, cimg_library::CImg<unsigned char> &image2, float alpha){
        if(image != NULL)
            image.draw_imageRGBA(x, y, image2, alpha);
    }

    void drawText(int x, int y, const char* text, int hexValue, int size, float alpha){
        if(image != NULL)
        {
            int r = ((hexValue >> 16) & 0xFF);
            int g = ((hexValue >> 8) & 0xFF);
            int b = ((hexValue) & 0xFF);
            const unsigned char color[] = { static_cast<unsigned char>(r),static_cast<unsigned char>(g),static_cast<unsigned char>(b) };
            image.draw_text(x, y, text, color, 0, alpha, size);
        }
    }

    void drawLine(int x0, int y0, int x1, int y1, int hexValue, float alpha, bool blend){
        if(image != NULL)
        {
            int r = ((hexValue >> 16) & 0xFF);
            int g = ((hexValue >> 8) & 0xFF);
            int b = ((hexValue) & 0xFF);
            if(blend)
            {
                const unsigned char color[] = { static_cast<unsigned char>(r),static_cast<unsigned char>(g),static_cast<unsigned char>(b) ,static_cast<unsigned char>(255)};
                image.draw_line(x0, y0, x1, y1, color, alpha);
            }
            else
            {
                const unsigned char color[] = { static_cast<unsigned char>(r),static_cast<unsigned char>(g),static_cast<unsigned char>(b) ,static_cast<unsigned char>(alpha*255)};
                image.draw_line(x0, y0, x1, y1, color, 1);
            }
        }
    }

    void drawFill(int x, int y, int width, int height, int hexValue, float alpha, bool blend){
        if(image != NULL)
        {
            int r = ((hexValue >> 16) & 0xFF);
            int g = ((hexValue >> 8) & 0xFF);
            int b = ((hexValue) & 0xFF);
            if(blend)
            {
                const unsigned char color[] = { static_cast<unsigned char>(r),static_cast<unsigned char>(g),static_cast<unsigned char>(b) ,static_cast<unsigned char>(255)};
                image.draw_rectangle(x, y, x+width, x+height, color, alpha);
            }
            else
            {
                const unsigned char color[] = { static_cast<unsigned char>(r),static_cast<unsigned char>(g),static_cast<unsigned char>(b) ,static_cast<unsigned char>(alpha*255)};
                image.draw_rectangle(x, y, x+width, x+height, color, 1);
            }
        }
    }

    void floodFill(int x, int y, int hexValue, float alpha, float tolerance, bool blend){
        if(image != NULL)
        {
            int r = ((hexValue >> 16) & 0xFF);
            int g = ((hexValue >> 8) & 0xFF);
            int b = ((hexValue) & 0xFF);
            if(blend)
            {
                const unsigned char color[] = { static_cast<unsigned char>(r),static_cast<unsigned char>(g),static_cast<unsigned char>(b) ,static_cast<unsigned char>(255)};
                image.draw_fill(x, y, color, alpha, tolerance, false);
            }
            else
            {
                const unsigned char color[] = { static_cast<unsigned char>(r),static_cast<unsigned char>(g),static_cast<unsigned char>(b) ,static_cast<unsigned char>(alpha*255)};
                image.draw_fill(x, y, color, 1, tolerance, false);
            }
        }
    }

private:
    const char* path_;
    float angle_;

public:
    cimg_library::CImg<unsigned char> image;
};

static int destructManager(lua_State* L)
{
    void *ptr = *(void**)lua_touserdata(L, 1);
    GReferenced* object = static_cast<GReferenced*>(ptr);
    GMediaManager *g = static_cast<GMediaManager*>(object->proxy());

    g->unref();

    return 0;
}

static int destructMedia(lua_State* L)
{
    void *ptr = *(void**)lua_touserdata(L, 1);
    GReferenced* object = static_cast<GReferenced*>(ptr);
    GMedia *g = static_cast<GMedia*>(object->proxy());

    g->unref();

    return 0;
}

static GMediaManager *getInstance(lua_State* L, int index)
{
    GReferenced *object = static_cast<GReferenced*>(g_getInstance(L, "MediaManager", index));
    GMediaManager *g = static_cast<GMediaManager*>(object->proxy());

    return g;
}

static GMedia *getMediaInstance(lua_State* L, int index)
{
    GReferenced *object = static_cast<GReferenced*>(g_getInstance(L, "Media", index));
    GMedia *g = static_cast<GMedia*>(object->proxy());

    return g;
}

static int isCameraAvailable(lua_State *L)
{
    GMediaManager *g = getInstance(L, 1);
    lua_pushboolean(L, (bool)g->isCameraAvailable());
    return 1;
}

static int takePicture(lua_State *L)
{
    GMediaManager *g = getInstance(L, 1);
    g->takePicture();
    return 0;
}

static int takeScreenshot(lua_State *L)
{
    GMediaManager *g = getInstance(L, 1);
    g->takeScreenshot();
    return 0;
}

static int getPicture(lua_State *L)
{
    GMediaManager *g = getInstance(L, 1);
    g->getPicture();
    return 0;
}

static int savePicture(lua_State *L)
{
    GMediaManager *g = getInstance(L, 1);
    const char* originalPath = luaL_checkstring(L, 2);
    std::string path(g_pathForFile(originalPath));
    if(file_exists(path.c_str()))
    {
        int drive = gpath_getPathDrive(originalPath);
        if(drive == 0)
            path = copyFile(path);
        g->savePicture(g_pathForFile(path.c_str()));
    }
    return 0;
}

static int playVideo(lua_State *L)
{
    GMediaManager *g = getInstance(L, 1);
    const char* path = g_pathForFile(luaL_checkstring(L, 2));
    bool force = false;
    if(lua_isnoneornil(L, 3) == 0)
        force = lua_toboolean(L, 3);
    if(file_exists(path))
        g->playVideo(path, force);
    return 0;
}

static int deleteFile(lua_State *L)
{
    const char* path = g_pathForFile(luaL_checkstring(L, 2));
    if(file_exists(path))
        remove(path);
    return 0;
}

static int init(lua_State *L)
{
    if(lua_isnumber(L, 1) && lua_isnumber(L,2))
    {
        GMedia *g = new GMedia(lua_tonumber(L, 1), lua_tonumber(L, 2));
        g_pushInstance(L, "Media", g->object());

        luaL_rawgetptr(L, LUA_REGISTRYINDEX, &keyWeak);
        lua_pushvalue(L, -2);
        luaL_rawsetptr(L, -2, g);
        lua_pop(L, 1);

        lua_pushvalue(L, -1);
        return 1;
    }
    else
    {
        const char* originalPath = luaL_checkstring(L, 1);
        std::string path(g_pathForFile(originalPath));
        if(file_exists(path.c_str()))
        {
            int drive = gpath_getPathDrive(originalPath);
            if(drive == 0)
                path = copyFile(path);
            GMedia *g = new GMedia(g_pathForFile(path.c_str()));
            g_pushInstance(L, "Media", g->object());

            luaL_rawgetptr(L, LUA_REGISTRYINDEX, &keyWeak);
            lua_pushvalue(L, -2);
            luaL_rawsetptr(L, -2, g);
            lua_pop(L, 1);

            lua_pushvalue(L, -1);
            return 1;
        }
        lua_pushstring( L , "File does not exist" );
        lua_error( L );
     }
    return 0;
}

static int getPath(lua_State *L)
{
    GMedia *g = getMediaInstance(L, 1);
    lua_pushstring(L, g->getPath());
    return 1;
}

static int getWidth(lua_State *L)
{
    GMedia *g = getMediaInstance(L, 1);
    lua_pushnumber(L, g->getWidth());
    return 1;
}

static int getPixel(lua_State *L)
{
    GMedia *g = getMediaInstance(L, 1);
    int x = lua_tonumber(L, 2);
    int y = lua_tonumber(L, 3);
    lua_pushnumber(L, g->getR(x, y));
    lua_pushnumber(L, g->getG(x, y));
    lua_pushnumber(L, g->getB(x, y));
    lua_pushnumber(L, g->getA(x, y));
    return 4;
}

static int setPixel(lua_State *L)
{
    GMedia *i = getMediaInstance(L, 1);
    int x = lua_tonumber(L, 2);
    int y = lua_tonumber(L, 3);
    if(lua_isnumber(L, 4) && lua_isnumber(L, 5) && lua_isnumber(L, 6))
    {
        int r = lua_tonumber(L, 4);
        int g = lua_tonumber(L, 5);
        int b = lua_tonumber(L, 6);
        float a = 1;
        if(lua_isnoneornil(L, 7) == 0)
            a = lua_tonumber(L, 7);
        bool blend = false;
        if(lua_isnoneornil(L, 8) == 0)
            blend = lua_toboolean(L, 8);
        i->setPixel(x, y, r, g, b, a, blend);
    }
    else if(lua_isnumber(L, 4))
    {
        int hexValue = lua_tonumber(L, 4);
        float a = 1;
        if(lua_isnoneornil(L, 5) == 0)
            a = lua_tonumber(L, 5);
        bool blend = false;
        if(lua_isnoneornil(L, 6) == 0)
            blend = lua_toboolean(L, 5);
        int r = ((hexValue >> 16) & 0xFF);
        int g = ((hexValue >> 8) & 0xFF);
        int b = ((hexValue) & 0xFF);
        i->setPixel(x, y, r, g, b, a, blend);
    }
    return 0;
}

static int getHeight(lua_State *L)
{
    GMedia *g = getMediaInstance(L, 1);
    lua_pushnumber(L, g->getHeight());
    return 1;
}

static int resizeWidth(lua_State *L)
{
    GMedia *g = getMediaInstance(L, 1);
    bool fixed = true;
    if(lua_isnoneornil(L, 3) == 0)
        fixed = lua_toboolean(L, 3);
    g->resizeWidth(lua_tonumber(L, 2), fixed);
    return 0;
}

static int resizeHeight(lua_State *L)
{
    GMedia *g = getMediaInstance(L, 1);
    bool fixed = true;
    if(lua_isnoneornil(L, 3) == 0)
        fixed = lua_toboolean(L, 3);
    g->resizeHeight(lua_tonumber(L, 2), fixed);
    return 0;
}

static int resize(lua_State *L)
{
    GMedia *g = getMediaInstance(L, 1);
    bool fixed = true;
    if(lua_isnoneornil(L, 4) == 0)
        fixed = lua_toboolean(L, 4);
    bool crop = false;
    if(lua_isnoneornil(L, 5) == 0)
        crop = lua_toboolean(L, 5);
    g->resize(lua_tonumber(L, 2), lua_tonumber(L, 3), crop, fixed);
    return 0;
}

static int crop(lua_State *L)
{
    GMedia *g = getMediaInstance(L, 1);
    g->crop(lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4), lua_tonumber(L, 5));
    return 0;
}

static int copy(lua_State *L)
{
    GMedia *g = getMediaInstance(L, 1);
    g->copy(g_pathForFile(luaL_checkstring(L, 2)));
    return 0;
}

static int save(lua_State *L)
{
    GMedia *g = getMediaInstance(L, 1);
    g->save();
    return 0;
}

static int setRotation(lua_State *L)
{
    GMedia *g = getMediaInstance(L, 1);
    g->setRotation(lua_tonumber(L, 2));
    return 0;
}

static int getRotation(lua_State *L)
{
    GMedia *g = getMediaInstance(L, 1);
    lua_pushnumber(L, g->getRotation());
    return 1;
}

static int flipHorizontal(lua_State *L)
{
    GMedia *g = getMediaInstance(L, 1);
    g->flip("x");
    return 0;
}

static int flipVertical(lua_State *L)
{
    GMedia *g = getMediaInstance(L, 1);
    g->flip("y");
    return 0;
}

static int drawImage(lua_State *L)
{
    GMedia *g = getMediaInstance(L, 1);
    int x = lua_tonumber(L, 2);
    int y = lua_tonumber(L, 3);
    float alpha = 1;
    if(lua_isnoneornil(L, 5) == 0)
        alpha = lua_tonumber(L, 5);
    if(lua_isstring(L, 4))
    {
        const char* originalPath = luaL_checkstring(L, 4);
        std::string path(g_pathForFile(originalPath));
        if(file_exists(path.c_str()))
        {
            int drive = gpath_getPathDrive(originalPath);
            if(drive == 0)
                path = copyFile(path);
            g->drawImage(x, y, g_pathForFile(path.c_str()), alpha);
        }
    }
    else
        g->drawImage(x, y, getMediaInstance(L, 4), alpha);
    return 0;
}

static int drawText(lua_State *L){
    GMedia *g = getMediaInstance(L, 1);
    int x = lua_tonumber(L, 2);
    int y = lua_tonumber(L, 3);
    const char* text = luaL_checkstring(L, 4);
    int hex = 0;
    if(lua_isnoneornil(L, 5) == 0)
        hex = lua_tonumber(L, 5);

    int size = 20;
    if(lua_isnoneornil(L, 6) == 0)
        size = lua_tonumber(L, 6);
    float alpha = 1.0;
    if(lua_isnoneornil(L, 7) == 0)
        alpha = lua_tonumber(L, 7);
    g->drawText(x, y, text, hex, size, alpha);
    return 0;
}

static int drawLine(lua_State *L){
    GMedia *g = getMediaInstance(L, 1);
    int x0 = lua_tonumber(L, 2);
    int y0 = lua_tonumber(L, 3);
    int x1 = lua_tonumber(L, 4);
    int y1 = lua_tonumber(L, 5);
    int hex = 0;
    if(lua_isnoneornil(L, 6) == 0)
        hex = lua_tonumber(L, 6);
    float alpha = 1.0;
    if(lua_isnoneornil(L, 7) == 0)
        alpha = lua_tonumber(L, 7);
    bool blend = false;
    if(lua_isnoneornil(L, 8) == 0)
        blend = lua_toboolean(L, 8);
    g->drawLine(x0, y0, x1, y1, hex, alpha, blend);
    return 0;
}

static int drawFill(lua_State *L){
    GMedia *g = getMediaInstance(L, 1);
    int x = lua_tonumber(L, 2);
    int y = lua_tonumber(L, 3);
    int width = lua_tonumber(L, 4);
    int height = lua_tonumber(L, 5);
    int hex = 0;
    if(lua_isnoneornil(L, 6) == 0)
        hex = lua_tonumber(L, 6);
    float alpha = 1.0;
    if(lua_isnoneornil(L, 7) == 0)
        alpha = lua_tonumber(L, 7);
    bool blend = false;
    if(lua_isnoneornil(L, 8) == 0)
        blend = lua_toboolean(L, 8);
    g->drawFill(x, y, width, height, hex, alpha, blend);
    return 0;
}

static int floodFill(lua_State *L){
    GMedia *g = getMediaInstance(L, 1);
    int x = lua_tonumber(L, 2);
    int y = lua_tonumber(L, 3);
    int hex = 0;
    if(lua_isnoneornil(L, 4) == 0)
        hex = lua_tonumber(L, 4);
    float alpha = 1.0;
    if(lua_isnoneornil(L, 5) == 0)
        alpha = lua_tonumber(L, 5);
    float tolerance = 0.0;
    if(lua_isnoneornil(L, 6) == 0)
        tolerance = lua_tonumber(L, 6);
    bool blend = false;
    if(lua_isnoneornil(L, 7) == 0)
        blend = lua_toboolean(L, 7);
    g->floodFill(x, y, hex, alpha, tolerance, blend);
    return 0;
}

static int trim(lua_State *L)
{
    GMedia *g = getMediaInstance(L, 1);
    if(lua_isnoneornil(L, 2) == 0)
    {
        g->autocrop(lua_tonumber(L, 2));
    }
    else
    {
        g->autocrop();
    }
    return 0;
}

static int loader(lua_State *L)
{
    const luaL_Reg functionlist_mngr[] = {
        {"isCameraAvailable", isCameraAvailable},
        {"takeScreenshot", takeScreenshot},
        {"takePicture", takePicture},
        {"getPicture", getPicture},
        {"postPicture", savePicture},
        {"playVideo", playVideo},
        {"deleteFile", deleteFile},
        {NULL, NULL},
    };

    const luaL_Reg functionlist_media[] = {
        {"new", init},
        {"getPath", getPath},
        {"getPixel", getPixel},
        {"setPixel", setPixel},
        {"getWidth", getWidth},
        {"getHeight", getHeight},
        {"resizeWidth", resizeWidth},
        {"resizeHeight", resizeHeight},
        {"resize", resize},
        {"setRotation", setRotation},
        {"getRotation", getRotation},
        {"trim", trim},
        {"crop", crop},
        {"copy", copy},
        {"save", save},
        {"flipHorizontal", flipHorizontal},
        {"flipVertical", flipVertical},
        {"drawImage", drawImage},
        {"drawText", drawText},
        {"drawLine", drawLine},
        {"drawFill", drawFill},
        {"floodFill", floodFill},
        {NULL, NULL},
    };

    g_createClass(L, "MediaManager", "EventDispatcher", NULL, destructManager, functionlist_mngr);
    g_createClass(L, "Media", NULL, NULL, destructMedia, functionlist_media);

    // create a weak table in LUA_REGISTRYINDEX that can be accessed with the address of keyWeak
    luaL_newweaktable(L, "v");
    luaL_rawsetptr(L, LUA_REGISTRYINDEX, &keyWeak);

    lua_getglobal(L, "Event");
    lua_pushstring(L, MEDIA_RECEIVE);
    lua_setfield(L, -2, "MEDIA_RECEIVE");
    lua_pushstring(L, MEDIA_CANCEL);
    lua_setfield(L, -2, "MEDIA_CANCEL");
    lua_pushstring(L, VIDEO_COMPLETE);
    lua_setfield(L, -2, "VIDEO_COMPLETE");
    lua_pop(L, 1);

    GMediaManager *g = new GMediaManager();
    g_pushInstance(L, "MediaManager", g->object());

    luaL_rawgetptr(L, LUA_REGISTRYINDEX, &keyWeak);
    lua_pushvalue(L, -2);
    luaL_rawsetptr(L, -2, g);
    lua_pop(L, 1);

    lua_pushvalue(L, -1);
    lua_setglobal(L, "mediamanager");

    return 1;
}

static void g_initializePlugin(lua_State *L)
{
    ::L = L;
    lua_getglobal(L, "package");
    lua_getfield(L, -1, "preload");

    lua_pushcfunction(L, loader);
    lua_setfield(L, -2, "media");

    lua_pop(L, 2);
}

static void g_deinitializePlugin(lua_State *L)
{
    ::L = NULL;
}

REGISTER_PLUGIN("GMedia", "1.0")
