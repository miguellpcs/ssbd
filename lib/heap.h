#ifndef heap_h
#define heap_h

#include "instance.h"

#include <stdint.h>

typedef struct heap
{
    Instance *instances;
    uint64_t count;
} Heap;

typedef int64_t (*compare_weight)(uint32_t a, uint32_t b);
int64_t cmp_heap_min(uint32_t, uint32_t);
int64_t cmp_heap_max(uint32_t, uint32_t);
void heapify_bottom_top(Heap *, int, compare_weight);
void heapify_top_bottom(Heap *, int, compare_weight);
void insert_min_heap(Heap *, Instance *);
void insert_max_heap(Heap *, Instance *);
Instance *pop_min(Heap *);
Instance *pop_max(Heap *);
void remove_at_min(Heap *heap, uint64_t index);
void remove_at_max(Heap *heap, uint64_t index);
float heap_sum_prob(Heap *, float);

int64_t cmp_heap_min(uint32_t a, uint32_t b)
{
    // if a < b, then a - b < 0
    // if a == b, then a - b == 0
    // if a > b, then a - b > 0
    return (int64_t)a - (int64_t)b;
}

int64_t cmp_heap_max(uint32_t a, uint32_t b)
{
    // if a < b, then b - a > 0
    // if a == b, then b - a == 0
    // if a > b, then b - a < 0
    return (int64_t)b - (int64_t)a;
}

// MARK: - Heap functions
void insert_heap(Heap *heap, Instance *instance, compare_weight cmp)
{
    Instance new_instance = {.val = instance->val, .weight = instance->weight};
    heap->instances[heap->count] = new_instance;
    heapify_bottom_top(heap, heap->count, cmp);
    heap->count++;

    return;
}
void insert_min_heap(Heap *heap, Instance *instance)
{
    insert_heap(heap, instance, cmp_heap_min);
}

void insert_max_heap(Heap *heap, Instance *instance)
{
    insert_heap(heap, instance, cmp_heap_max);
}

void heapify_bottom_top(Heap *heap, int index, compare_weight cmp)
{
    Instance temp;
    int parent_node = (index - 1) / 2;

    uint32_t new_val = heap->instances[index].weight;
    uint32_t parent_val = heap->instances[parent_node].weight;

    if (cmp(new_val, parent_val) < 0)
    {
        temp = heap->instances[parent_node];
        heap->instances[parent_node] = heap->instances[index];
        heap->instances[index] = temp;
        heapify_bottom_top(heap, parent_node, cmp);
    }
}

uint64_t check_heap(Heap *heap, compare_weight cmp)
{
    uint64_t candidate = heap->instances[0].weight;
    for (int i = 1; i < heap->count; i++)
    {
        if (cmp(candidate, heap->instances[i].weight) > 0)
        {
            candidate = heap->instances[i].weight;
        }
    }
    return candidate;
}

uint64_t check_max_heap(Heap *heap)
{
    return check_heap(heap, cmp_heap_max);
}

uint64_t check_min_heap(Heap *heap)
{
    return check_heap(heap, cmp_heap_min);
}

void heapify_top_bottom(Heap *heap, int parent_node, compare_weight cmp)
{
    int left = parent_node * 2 + 1;
    int right = parent_node * 2 + 2;
    int candidate;
    Instance temp;

    if (left >= heap->count || left < 0)
        left = -1;
    if (right >= heap->count || right < 0)
        right = -1;

    if (left != -1 && cmp(heap->instances[left].weight, heap->instances[parent_node].weight) < 0)
        candidate = left;
    else
        candidate = parent_node;
    if (right != -1 && cmp(heap->instances[right].weight, heap->instances[candidate].weight) < 0)
        candidate = right;

    if (candidate != parent_node)
    {
        temp = heap->instances[candidate];
        heap->instances[candidate] = heap->instances[parent_node];
        heap->instances[parent_node] = temp;

        heapify_top_bottom(heap, candidate, cmp);
    }
}

Instance *pop(Heap *heap, compare_weight cmp)
{
    if (heap->count == 0)
    {
        return NULL;
    }

    Instance *pop = check_calloc(1, sizeof(Instance));
    pop->val = heap->instances[0].val;
    pop->weight = heap->instances[0].weight;
    heap->instances[0] = heap->instances[heap->count - 1];
    heap->count--;
    heapify_top_bottom(heap, 0, cmp);
    return pop;
}

Instance *pop_min(Heap *heap)
{
    return pop(heap, cmp_heap_min);
}

Instance *pop_max(Heap *heap)
{
    return pop(heap, cmp_heap_max);
}

void remove_at(Heap *heap, uint64_t index, compare_weight cmp)
{
    if (heap->count == 0)
    {
        return;
    }
    heap->instances[index] = heap->instances[0];
    heap->count--;
    heapify_bottom_top(heap, index, cmp);
    return;
}

void remove_at_min(Heap *heap, uint64_t index)
{
    remove_at(heap, index, cmp_heap_min);
}

void remove_at_max(Heap *heap, uint64_t index)
{
    remove_at(heap, index, cmp_heap_max);
}

float heap_sum_prob(Heap *heap, float tal)
{
    float total = 0.0f;

    for (uint64_t i = 0; i < heap->count; i++)
    {
        float prob = 1 - (heap->instances[i].weight / tal);
        if (tal == 0.0f)
        {
            prob = 0;
        }
        total += prob;
    }

    return total;
}

#endif