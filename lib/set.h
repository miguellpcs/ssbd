#ifndef set_h
#define set_h

#include "memory.h"
#include "sentinel.h"

typedef struct
{
    Sentinel **list;
    unsigned int size;
    unsigned int count;
} Set;

Set *set_init(unsigned int size)
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
    if (!set_is_present(set, value))
    {
        unsigned int h = hash(value) % set->size;
        sentinel_push(set->list[h], new_node(value));
        set->count++;
    }
}

void set_del(Set *set, unsigned int value)
{
    unsigned int h = hash(value) % set->size;
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