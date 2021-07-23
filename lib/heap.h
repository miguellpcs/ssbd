#ifndef heap_h
#define heap_h

#include "instance.h"

typedef struct heap
{
    Instance *instances;
    int count;
} Heap;

typedef int (*compare_weight)(int a, int b);
int cmp_heap_min(int, int);
int cmp_heap_max(int, int);
void heapify_bottom_top(Heap *, int, compare_weight);
void heapify_top_bottom(Heap *, int, compare_weight);
void insert_min_heap(Heap *, Instance *);
void insert_max_heap(Heap *, Instance *);
void pop_min(Heap *);
void pop_max(Heap *);
void remove_at_min(Heap *heap, int index);
void remove_at_max(Heap *heap, int index);
float heap_sum_prob(Heap *, float);

int cmp_heap_min(int a, int b)
{
    return a - b;
}

int cmp_heap_max(int a, int b)
{
    return b - a;
}

// MARK: - Heap functions
void insert_heap(Heap *heap, Instance *instance, compare_weight cmp)
{
    heap->instances[heap->count] = *instance;
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

    int new_val = heap->instances[index].weight;
    int parent_val = heap->instances[parent_node].weight;

    if (cmp(parent_val, new_val) > 0)
    {

        temp = heap->instances[parent_node];
        heap->instances[parent_node] = heap->instances[index];
        heap->instances[index] = temp;
        heapify_bottom_top(heap, parent_node, cmp);

        return;
    }
}

void heapify_top_bottom(Heap *heap, int parent_node, compare_weight cmp)
{
    int left = parent_node * 2 + 1;
    int right = parent_node * 2 + 2;
    int min;
    Instance temp;

    if (left >= heap->count || left < 0)
        left = -1;
    if (right >= heap->count || right < 0)
        right = -1;

    if (left != -1 && cmp(heap->instances[left].weight, heap->instances[parent_node].weight) < 0)
        min = left;
    else
        min = parent_node;
    if (right != -1 && cmp(heap->instances[right].weight, heap->instances[min].weight) < 0)
        min = right;

    if (min != parent_node)
    {
        temp = heap->instances[min];
        heap->instances[min] = heap->instances[parent_node];
        heap->instances[parent_node] = temp;

        heapify_top_bottom(heap, min, cmp);
    }
}

void pop(Heap *heap, compare_weight cmp)
{
    if (heap->count == 0)
    {
        return;
    }

    heap->instances[0] = heap->instances[heap->count - 1];
    heap->count--;
    heapify_top_bottom(heap, 0, cmp);
    return;
}

void pop_min(Heap *heap)
{
    pop(heap, cmp_heap_min);
}

void pop_max(Heap *heap)
{
    pop(heap, cmp_heap_max);
}

void remove_at(Heap *heap, int index, compare_weight cmp)
{
    if (heap->count == 0)
    {
        return;
    }

    heap->instances[index] = heap->instances[heap->count - 1];
    heap->count--;
    heapify_top_bottom(heap, index, cmp);
    return;
}

void remove_at_min(Heap *heap, int index)
{
    remove_at(heap, index, cmp_heap_min);
}

void remove_at_max(Heap *heap, int index)
{
    remove_at(heap, index, cmp_heap_max);
}

float heap_sum_prob(Heap *heap, float tal)
{
    float total = 0.0f;

    for (int i = 0; i < heap->count; i++)
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