#ifndef GCOUNTLY_H
#define GCOUNTLY_H

#include <gglobal.h>

#ifdef __cplusplus
extern "C" {
#endif

void gcountly_StartSession(const char *apiKey);
void gcountly_LogEvent(const char *eventName, int count, double sum, const char **parameters);
    
#ifdef __cplusplus
}
#endif

#endif
