#ifndef sentinel_h
#define sentinel_h

#include <stdlib.h>

#include "memory.h"

typedef struct Node
{
    unsigned int element;
    struct Node *next;
    struct Node *before;
} Node;

typedef struct Sentinel
{
    Node *head;
    Node *tail;
    unsigned int length;

    void (*insert)(struct Sentinel *self, Node *t, Node *x);
    unsigned int (*del)(struct Sentinel *self, Node *t);
    Node *(*search)(struct Sentinel *self, unsigned int element);

    void (*push)(struct Sentinel *self, Node *t);
    unsigned int (*pop)(struct Sentinel *self);

    void (*enqueue)(struct Sentinel *self, Node *t);
    unsigned int (*dequeue)(struct Sentinel *self);
} Sentinel;

unsigned int get_length(Sentinel *);
Sentinel *sentinel_init(void);
Node *new_node(unsigned int);
void free_sentinel(Sentinel *);

void sentinel_insert(Sentinel *, Node *, Node *);
unsigned int sentinel_delete(Sentinel *, Node *);
Node *sentinel_search(Sentinel *, unsigned int);

void sentinel_push(Sentinel *, Node *);
unsigned int sentinel_pop(Sentinel *);

void sentinel_enqueue(Sentinel *, Node *);
unsigned int sentinel_dequeue(Sentinel *);

unsigned int get_length(Sentinel *self) { return self->length; }

Sentinel *sentinel_init(void)
{
    Sentinel *tmp = (Sentinel *)check_malloc(sizeof(Sentinel));
    tmp->head = (Node *)check_malloc(sizeof(Node));
    tmp->tail = (Node *)check_malloc(sizeof(Node));
    tmp->head->next = tmp->tail->next = NULL;
    tmp->head->before = tmp->tail->before = NULL;

    tmp->insert = sentinel_insert;
    tmp->del = sentinel_delete;
    tmp->search = sentinel_search;

    tmp->push = sentinel_push;
    tmp->pop = sentinel_pop;

    tmp->enqueue = sentinel_enqueue;
    tmp->dequeue = sentinel_dequeue;

    tmp->length = 0;

    return tmp;
}

Node *new_node(unsigned int value)
{
    Node *tmp = (Node *)check_malloc(sizeof(Node));
    tmp->next = NULL;
    tmp->before = NULL;
    tmp->element = value;

    return tmp;
}

void free_sentinel(Sentinel *self)
{
    Node *tmp = self->head->next;
    while (tmp != NULL)
    {
        if (self->head->next->next != NULL)
        {
            tmp = self->head->next->next;
        }
        else
        {
            tmp = NULL;
        }
        check_free(self->head->next);
    }
    check_free(self->head);
    check_free(self->tail);
    check_free(self);
}

void sentinel_insert(Sentinel *self, Node *tmp, Node *x)
{
    if (self->length == 0)
    {
        self->head->next = self->tail->next = tmp;
    }
    else if (x == self->tail->next)
    {
        tmp->before = self->tail->next;
        self->tail->next->next = tmp;
        self->tail->next = tmp;
    }
    else
    {
        tmp->before = x;
        tmp->next = x->next;
        x->next = tmp;
        tmp->next->before = tmp;
    }

    self->length++;
}

unsigned int sentinel_delete(Sentinel *self, Node *x)
{
    Node *tmp;
    if (self->head->next == NULL || x->next == NULL || self->length == 0)
        return 0;
    else
    {
        tmp = x->next;
        if (tmp->next == NULL)
        {
            self->tail->next = x;
            x->next = NULL;
        }
        else
        {
            x->next = tmp->next;
            tmp->next->before = x;
        }
        self->length--;
    }
    unsigned int value = tmp->element;
    check_free(tmp);
    return value;
}

Node *sentinel_search(Sentinel *self, unsigned int value)
{
    Node *tmp = self->head->next;

    for (; tmp && tmp->element != value;)
    {
        if (tmp->next)
            tmp = tmp->next;
        else
            return NULL;
    }

    return tmp;
}

void sentinel_push(Sentinel *self, Node *tmp)
{
    sentinel_insert(self, tmp, self->head->next);
}

unsigned int sentinel_pop(Sentinel *self)
{
    return sentinel_delete(self, self->head->next);
}

void sentinel_enqueue(Sentinel *self, Node *tmp)
{
    sentinel_insert(self, tmp, self->head->next);
}

unsigned int sentinel_dequeue(Sentinel *self)
{
    return sentinel_delete(self, self->tail->next);
}

#endif // sentinel_h
