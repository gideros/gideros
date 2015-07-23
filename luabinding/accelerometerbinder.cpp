#include "accelerometerbinder.h"
#include "platform.h"
#include "stackchecker.h"
#include <refptr.h>
#include <ginput.h>
#include <stdexcept>

class GGAccelerometer : public GReferenced
{
public:
    GGAccelerometer()
    {
        isStarted_ = false;
    }

    ~GGAccelerometer()
    {
        stop();
    }

    void start()
    {
        if (isStarted_)
            return;

        ginput_startAccelerometer();;
        isStarted_ = true;
    }

    void stop()
    {
        if (!isStarted_)
            return;

        ginput_stopAccelerometer();;
        isStarted_ = false;
    }

    void getAcceleration(double *x, double *y, double *z)
    {
        ginput_getAcceleration(x, y, z);
    }

private:
    bool isStarted_;
};

AccelerometerBinder::AccelerometerBinder(lua_State* L)
{
	Binder binder(L);

	static const luaL_Reg functionList[] = {
        {"isAvailable", isAvailable},
        {"start", start},
        {"stop", stop},
        {"getAcceleration", getAcceleration},
		{NULL, NULL},
	};

    binder.createClass("Accelerometer", NULL, create, destruct, functionList);
}

int AccelerometerBinder::create(lua_State* L)
{
    Binder binder(L);

    GGAccelerometer *accelerometer = new GGAccelerometer;
    binder.pushInstance("Accelerometer", accelerometer);

    return 1;
}

int AccelerometerBinder::destruct(lua_State* L)
{
    void* ptr = *(void**)lua_touserdata(L, 1);
    GGAccelerometer *accelerometer = static_cast<GGAccelerometer*>(ptr);
    accelerometer->unref();

    return 0;
}

int AccelerometerBinder::isAvailable(lua_State* L)
{
    Binder binder(L);

    lua_pushboolean(L, ginput_isAccelerometerAvailable());

    return 1;
}

int AccelerometerBinder::start(lua_State* L)
{
    Binder binder(L);

    GGAccelerometer *accelerometer = static_cast<GGAccelerometer*>(binder.getInstance("Accelerometer", 1));
    accelerometer->start();

    return 0;
}

int AccelerometerBinder::stop(lua_State* L)
{
    Binder binder(L);

    GGAccelerometer *accelerometer = static_cast<GGAccelerometer*>(binder.getInstance("Accelerometer", 1));
    accelerometer->stop();

    return 0;
}

int AccelerometerBinder::getAcceleration(lua_State* L)
{
    Binder binder(L);

    GGAccelerometer *accelerometer = static_cast<GGAccelerometer*>(binder.getInstance("Accelerometer", 1));
    double x, y, z;
    accelerometer->getAcceleration(&x, &y, &z);

    lua_pushnumber(L, x);
    lua_pushnumber(L, y);
    lua_pushnumber(L, z);

    return 3;
}
