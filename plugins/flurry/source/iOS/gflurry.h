#ifndef GFLURRY_H
#define GFLURRY_H

#include <gglobal.h>

#ifdef __cplusplus
extern "C" {
#endif

void gflurry_StartSession(const char *apiKey);
void gflurry_LogEvent(const char *eventName, const char **parameters, int timed);
void gflurry_EndTimedEvent(const char *eventName, const char **parameters);
void glurry_setSessionContinueSeconds(int seconds);
void gflurry_setGender(const char *gender);
void gflurry_setUserID(const char *userID);
void gflurry_logError(const char *errorID, const char *message);
#ifdef __cplusplus
}
#endif

#endif
