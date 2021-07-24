#ifndef memory_h
#define memory_h

#include <assert.h>
#include <stdlib.h>

void *check_malloc(size_t size)
{
    void *mem = malloc(size);
    assert(mem != NULL);
    return mem;
}

void *check_realloc(void *ptr, size_t size)
{
    void *mem = realloc(ptr, size);
    assert(mem != NULL);
    return mem;
}

void check_free(void *ptr)
{
    if (ptr != NULL)
    {
        free(ptr);
        ptr = NULL;
    }
}

#endif