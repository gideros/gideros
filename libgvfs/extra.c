#include <stdlib.h>

int     g__isthreaded    = 1;

void __atexit_register_cleanup(void (*func)(void))
{
    atexit(func);
}
