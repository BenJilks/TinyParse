#include "debug.h"

#if DEBUG

#include <stdio.h>

static int scope = 0;

void debug_prefix()
{
    int i;

    for (i = 0; i < scope; i++)
        printf("\t");
}

void debug_start_scope()
{
    scope += 1;
}

void debug_end_scope()
{
    scope -= 1;
    
    if (scope < 0)
        scope = 0;
}

#endif
