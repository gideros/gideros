#include "custom_alloc.h"
#include "stdlib.h"

void* allocF(void *ud, void *ptr, size_t osize, size_t nsize)
{
    (void)ud;
    (void)osize;
    void *ret=NULL;
     if (nsize == 0)
    {
    	if (ptr)
            free(ptr);
    }
    else if (ptr==NULL)
        ret=malloc(nsize);
    else
        ret=realloc(ptr, nsize);

    return ret;
}

void* getalloc(lua_State* L, void* allocUD)
{
    return &allocF;
}