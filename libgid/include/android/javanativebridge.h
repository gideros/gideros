#ifndef JAVANATIVEBRIDGE_H
#define JAVANATIVEBRIDGE_H

#include <jni.h>
#include <vector>
#include <string>

void jnb_setJavaVM(JavaVM* vm);
std::vector<std::string> jnb_getLocalIPs();

extern "C" {
JavaVM *g_getJavaVM();
JNIEnv *g_getJNIEnv();
}

#endif
