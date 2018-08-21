#ifndef _CUSTOM_ALLOC_H
#define _CUSTOM_ALLOC_H

#include "stddef.h"
#include "lua.h"

/*
 *   Gideros uses a TLSF ( http://www.gii.upv.es/tlsf/ ) based memory pool, which is great 
 *   for performance, but is not thread safe/aware so use a different allocator  for
 *   for Lua states in Lanes threads.
 *
 */

// this is called first before creating a Lua state if CUSTOM_ALLOCATOR_WITH_PREP is defined
void* getalloc(lua_State* L, void* allocUD);

// this is used as the allocator for new Lua states when either CUSTOM_ALLOCATOR or 
// CUSTOM_ALLOCATOR_WITH_PREP is defined
void* allocF(void *ud, void *ptr, size_t osize, size_t nsize);


#endif