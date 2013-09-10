#ifndef GFLURRY_H
#define GFLURRY_H

#include <gglobal.h>

#ifdef __cplusplus
extern "C" {
#endif

void gflurry_StartSession(const char *apiKey);
void gflurry_LogEvent(const char *eventName, const char **parameters, int timed);
void gflurry_EndTimedEvent(const char *eventName, const char **parameters);
    
#ifdef __cplusplus
}
#endif

#endif
