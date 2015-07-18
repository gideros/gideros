#include "gyroscopebinder.h"
#include "platform.h"
#include "stackchecker.h"
#include <refptr.h>
#include <ginput.h>
#include <stdexcept>

class GGGyroscope : public GReferenced
{
public:
    GGGyroscope()
    {
        isStarted_ = false;
    }

    ~GGGyroscope()
    {
        stop();
    }

    void start()
    {
        if (isStarted_)
            return;

        ginput_startGyroscope();
        isStarted_ = true;
    }

    void stop()
    {
        if (!isStarted_)
            return;

        ginput_stopGyroscope();
        isStarted_ = false;
    }

    void getRotationRate(double *x, double *y, double *z)
    {
        ginput_getGyroscopeRotationRate(x, y, z);
    }

private:
    bool isStarted_;
};

GyroscopeBinder::GyroscopeBinder(lua_State* L)
{
    Binder binder(L);

    static const luaL_Reg functionList[] = {
        {"isAvailable", isAvailable},
        {"start", start},
        {"stop", stop},
        {"getRotationRate", getRotationRate},
        {NULL, NULL},
    };

    binder.createClass("Gyroscope", NULL, create, destruct, functionList);
}

int GyroscopeBinder::create(lua_State* L)
{
    Binder binder(L);

    GGGyroscope *gyroscope = new GGGyroscope;
    binder.pushInstance("Gyroscope", gyroscope);

    return 1;
}

int GyroscopeBinder::destruct(lua_State* L)
{
    void* ptr = *(void**)lua_touserdata(L, 1);
    GGGyroscope *gyroscope = static_cast<GGGyroscope*>(ptr);
    gyroscope->unref();

    return 0;
}

int GyroscopeBinder::isAvailable(lua_State* L)
{
    Binder binder(L);

    lua_pushboolean(L, ginput_isGyroscopeAvailable());

    return 1;
}

int GyroscopeBinder::start(lua_State* L)
{
    Binder binder(L);

    GGGyroscope *gyroscope = static_cast<GGGyroscope*>(binder.getInstance("Gyroscope", 1));
    gyroscope->start();

    return 0;
}

int GyroscopeBinder::stop(lua_State* L)
{
    Binder binder(L);

    GGGyroscope *gyroscope = static_cast<GGGyroscope*>(binder.getInstance("Gyroscope", 1));
    gyroscope->stop();

    return 0;
}

int GyroscopeBinder::getRotationRate(lua_State* L)
{
    Binder binder(L);

    GGGyroscope *gyroscope = static_cast<GGGyroscope*>(binder.getInstance("Gyroscope", 1));
    double x, y, z;
    gyroscope->getRotationRate(&x, &y, &z);

    lua_pushnumber(L, x);
    lua_pushnumber(L, y);
    lua_pushnumber(L, z);

    return 3;
}
