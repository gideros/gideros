#pragma once
#include "gideros.h"

#define TO_OBJ(x,y) x* instance = static_cast<x*>(g_getInstance(L, x::class_name.c_str(), 1)); return instance->y(L)
#define UNUSED(x) (void)x
