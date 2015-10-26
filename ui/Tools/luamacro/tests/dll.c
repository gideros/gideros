// luam -C -lcexport dll.c
// expands this file into pukka C and creates/updates dll.h

#include "dll.h"

export {
typedef struct {
   int ival;
} MyStruct;
}

// yes we could use #define here, but it's sometimes useful to have another level
// of macro substitution
def_ alloc(T) (T*)malloc(sizeof(T))

// Plus, LuaMacro can do operator replacements. This is Ruby-style 'field' access
def_ @ self->

export MyStruct *create() {
    return alloc(MyStruct);
}

def_ This MyStruct *self

export int one(This) {
    return @ival + 1
}

export int two(This) {
    return 2*@ival;
}

export void set(This,int i) {
    @ival = i;
}


