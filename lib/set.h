#ifndef set_h
#define set_h

#include "hash.h"
#include "memory.h"
#include "sentinel.h"

#include <stdint.h>

typedef struct
{
    Sentinel **list;
    uint32_t size;
    uint32_t count;
} Set;

Set *set_init(uint32_t size)
{
    Set *tmp = (Set *)check_malloc(sizeof(Set));
    tmp->size = size;
    tmp->count = 0;
    tmp->list = (Sentinel **)check_malloc(sizeof(Sentinel *) * size);
    for (int i = 0; i < size; i++)
    {
        tmp->list[i] = sentinel_init();
    }

    return tmp;
}

int set_is_present(Set *set, const char *value)
{
    uint32_t h = murmurhash(value, strlen(value), 0) % set->size;
    return sentinel_search(set->list[h], value) != NULL;
}

void set_insert(Set *set, const char *value)
{
    if (!set_is_present(set, value))
    {
        uint32_t h = murmurhash(value, strlen(value), 0) % set->size;
        sentinel_push(set->list[h], new_node(value));
        set->count++;
    }
}

void set_del(Set *set, const char *value)
{
    uint32_t h = murmurhash(value, strlen(value), 0) % set->size;
    Node *node = sentinel_search(set->list[h], value);
    if (node != NULL)
    {
        sentinel_delete(set->list[h], node);
        set->count--;
    }
}

void free_set(Set *set)
{
    for (int i = 0; i < set->size; i++)
    {
        sentinel_destroy(set->list[i]);
    }
    check_free(set);
}

#endif