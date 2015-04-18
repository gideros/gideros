#ifndef GTIMER_H
#define GTIMER_H

#ifdef __cplusplus
extern "C" {
#endif

#define GTIMER_TIMER 0
#define GTIMER_COMPLETE 1

typedef struct gtimer_Timer
{
} gtimer_Timer;

typedef struct gtimer_Complete
{
} gtimer_Complete;

typedef void(*gtimer_Callback)(int id, int type, void* data, void* udata);

void gtimer_init();
void gtimer_cleanup();

double gtimer_clock();

int gtimer_create(double delay, int repeatCount, gtimer_Callback callback, void* udata);
void gtimer_stop(int id);
int gtimer_getCurrentCount(int id);
void gtimer_pauseAll();
void gtimer_resumeAll();
void gtimer_stopAll();


void gtimer_tick();

#ifdef __cplusplus
}
#endif

#endif
