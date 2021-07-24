#ifndef set_h
#define set_h

#include "memory.h"
#include "sentinel.h"

typedef struct
{
    Sentinel **list;
    unsigned int size;
} Set;

Set *set_init(unsigned int size)
{
    Set *tmp = (Set *)check_malloc(sizeof(Set));
    tmp->size = size;
    tmp->list = (Sentinel **)check_malloc(sizeof(Sentinel *) * size);
    for (int i = 0; i < size; i++)
    {
        tmp->list[i] = sentinel_init();
    }

    return tmp;
}

unsigned int hash(unsigned int x)
{
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = (x >> 16) ^ x;
    return x;
}

int set_is_present(Set *set, unsigned int value)
{
    unsigned int h = hash(value) % set->size;
    return sentinel_search(set->list[h], value) != NULL;
}

void set_insert(Set *set, unsigned int value)
{
    unsigned int h = hash(value) % set->size;
    sentinel_push(set->list[h], new_node(value));
}

void set_del(Set *set, unsigned int value)
{
    unsigned int h = hash(value) % set->size;
    Node *node = sentinel_search(set->list[h], value);
    if (node != NULL)
    {
        sentinel_delete(set->list[h], node);
    }
}

void free_set(Set *set)
{
    for (int i = 0; i < set->size; i++)
    {
        free_sentinel(set->list[i]);
    }
    check_free(set);
}

#endif