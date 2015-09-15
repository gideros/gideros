#include "gtimer.h"
#include <stdio.h>
#include <map>
#include <vector>

namespace g_private {

class StopWatch
{
public:
    StopWatch();

    double clock();
    void pause();
    void resume();
    bool isPaused() const;

private:
    bool isRunning_;
    double last_;
    double total_;
};

StopWatch::StopWatch()
{
    isRunning_ = true;
    last_ = gtimer_clock();
    total_ = 0;
}

double StopWatch::clock()
{
    double current = gtimer_clock();
    if (isRunning_ == true)
        total_ += current - last_;
    last_ = current;

    return total_;
}

void StopWatch::pause()
{
    clock();
    isRunning_ = false;
    clock();
}

void StopWatch::resume()
{
    clock();
    isRunning_ = true;
    clock();
}

bool StopWatch::isPaused() const
{
    return !isRunning_;
}

class TimerManager
{
public:
    TimerManager()
    {
        nextid_ = 1;
    }

    int create(double delay, int repeatCount, gtimer_Callback callback, void* udata)
    {
        TimerElement element;
        element.delay = delay;
        element.repeatCount = repeatCount;
        element.callback = callback;
        element.udata = udata;
        element.currentCount = 0;

        map_[nextid_] = element;

        queue_[gtimer_clock() + delay].push_back(nextid_);

        return nextid_++;
    }

    void stop(int id)
    {
        map_.erase(id);
    }

    void tick()
    {
        double clock = stopWatch_.clock();

        while (queue_.empty() == false && queue_.begin()->first < clock)
        {
            std::vector<int> ids = queue_.begin()->second;

            queue_.erase(queue_.begin());

            for (std::size_t i = 0; i < ids.size(); ++i)
            {
                int id = ids[i];

                std::map<int, TimerElement>::iterator iter;
                TimerElement* element;

                iter = map_.find(id);
                if (iter == map_.end())
                    continue;
                element = &iter->second;

                element->currentCount++;

                gtimer_Timer timer;
                element->callback(id, GTIMER_TIMER, &timer, element->udata);

                iter = map_.find(id);
                if (iter == map_.end())
                    continue;
                element = &iter->second;

                if (element->repeatCount != 0 && element->currentCount >= element->repeatCount)
                {
                    gtimer_Complete complete;
                    element->callback(id, GTIMER_COMPLETE, &complete, element->udata);

                    map_.erase(id);
                }
                else
                {
                    queue_[clock + element->delay].push_back(id);
                }
            }
        }
    }

    int getCurrentCount(int id)
    {
        std::map<int, TimerElement>::iterator iter;

        iter = map_.find(id);
        if (iter == map_.end())
            return 0;

        return iter->second.currentCount;
    }

    void pauseAll()
    {
        stopWatch_.pause();
    }

    void resumeAll()
    {
        stopWatch_.resume();
    }

    void stopAll()
    {
        queue_.clear();
        map_.clear();
    }

private:
    struct TimerElement
    {
        double delay;
        int repeatCount;
        int currentCount;
        gtimer_Callback callback;
        void* udata;
    };

    std::map<double, std::vector<int> > queue_;
    std::map<int, TimerElement> map_;
    int nextid_;
    StopWatch stopWatch_;
};

}

using namespace g_private;

static TimerManager* s_manager = NULL;

extern "C" {

void gtimer_init()
{
    s_manager = new TimerManager;
}

void gtimer_cleanup()
{
    delete s_manager;
    s_manager = NULL;
}

int gtimer_create(double delay, int repeatCount, gtimer_Callback callback, void* udata)
{
    return s_manager->create(delay, repeatCount, callback, udata);
}

void gtimer_stop(int id)
{
    s_manager->stop(id);
}

void gtimer_pauseAll()
{
    s_manager->pauseAll();
}

void gtimer_resumeAll()
{
    s_manager->resumeAll();
}

void gtimer_stopAll()
{
    s_manager->stopAll();
}

void gtimer_tick()
{
    s_manager->tick();
}

int gtimer_getCurrentCount(int id)
{
    return s_manager->getCurrentCount(id);
}

#if defined(_WIN32)
#include <Windows.h>
double gtimer_clock()
{
    static LARGE_INTEGER freq;
    static LARGE_INTEGER start;
    static bool init = false;

    if (init == false)
    {
        QueryPerformanceFrequency(&freq);
        QueryPerformanceCounter(&start);
        init = true;
    }

    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);

    LONGLONG delta = li.QuadPart - start.QuadPart;

    long double result = (long double)delta / (long double)freq.QuadPart;

    return (double)result;
}
#elif defined(__APPLE__)
#import <CoreFoundation/CoreFoundation.h>
double gtimer_clock()
{
    static double begin = CFAbsoluteTimeGetCurrent();
    return CFAbsoluteTimeGetCurrent() - begin;
}
#elif defined(__ANDROID__)
static double nanoTime()
{
    struct timespec t;
    if(clock_gettime(CLOCK_MONOTONIC, &t))
        return 0;
    return t.tv_sec + t.tv_nsec * 1e-9;
}
double gtimer_clock()
{
    static double begin = nanoTime();
    return nanoTime() - begin;
}
#elif defined(STRICT_LINUX)
#include <time.h>
static double nanoTime()
{
    struct timespec t;
    if(clock_gettime(CLOCK_MONOTONIC, &t))
        return 0;
    return t.tv_sec + t.tv_nsec * 1e-9;
}
double gtimer_clock()
{
    static double begin = nanoTime();
    return nanoTime() - begin;
}
#endif

}
