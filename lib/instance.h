#ifndef instance_h
#define instance_h

#include <stdint.h>

typedef struct instance
{
    void *val;
    uint32_t weight;
    void *extra;
} Instance;
#endif
