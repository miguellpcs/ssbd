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
} Sentinel;

unsigned int get_length(Sentinel *);
Sentinel *sentinel_init(void);
Node *new_node(unsigned int);
void sentinel_destroy(Sentinel *);

Node *sentinel_search(Sentinel *, unsigned int);

void sentinel_push(Sentinel *, Node *);
unsigned int sentinel_pop(Sentinel *);

#define SENTINEL_FOREACH(L, S, M, V) \
    Node *_node = NULL;              \
    Node *V = NULL;                  \
    for (V = _node = L->S; _node != NULL; V = _node = _node->M)

unsigned int get_length(Sentinel *self)
{
    return self->length;
}

Sentinel *sentinel_init(void)
{
    return check_calloc(1, sizeof(Sentinel));
}

Node *new_node(unsigned int value)
{
    Node *tmp = (Node *)check_calloc(1, sizeof(Node));
    tmp->element = value;
    return tmp;
}

void sentinel_destroy(Sentinel *self)
{
    SENTINEL_FOREACH(self, head, next, cur)
    {
        if (cur->before)
            check_free(cur->before);
    }

    check_free(self->tail);
    check_free(self);
}

Node *sentinel_search(Sentinel *self, unsigned int value)
{
    SENTINEL_FOREACH(self, head, next, cur)
    {
        if (cur->element == value)
        {
            return cur;
        }
    }
    return NULL;
}

void sentinel_push(Sentinel *self, Node *tmp)
{
    if (self->tail == NULL)
    {
        self->head = tmp;
        self->tail = tmp;
    }
    else
    {
        self->tail->next = tmp;
        tmp->before = self->tail;
        self->tail = tmp;
    }

    self->length++;
}

unsigned int sentinel_delete(Sentinel *self, Node *node)
{
    if (node == self->head && node == self->tail)
    {
        self->head = NULL;
        self->tail = NULL;
    }
    else if (node == self->head)
    {
        self->head = node->next;
        self->head->before = NULL;
    }
    else if (node == self->tail)
    {
        self->tail = node->before;
        self->tail->next = NULL;
    }
    else
    {
        Node *next = node->next;
        Node *before = node->before;
        next->before = before;
        before->next = next;
    }

    self->length--;
    unsigned int value = node->element;
    check_free(node);

    return value;
}

unsigned int sentinel_pop(Sentinel *self)
{
    Node *node = self->tail;
    return node != NULL ? sentinel_delete(self, node) : 0;
}

#endif // sentinel_h
