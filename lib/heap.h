#ifndef heap_h
#define heap_h

#include "instance.h"

typedef struct heap
{
    Instance *instances;
    int count;
} Heap;
void heapify_bottom_top(Heap *, int);
void heapify_top_bottom(Heap *, int);
void insert_min_heap(Heap *, Instance *);
void pop_min(Heap *);

// MARK: - Heap functions
void insert_min_heap(Heap *heap, Instance *instance)
{
    heap->instances[heap->count] = *instance;
    heapify_bottom_top(heap, heap->count);
    heap->count++;

    return;
}

void heapify_bottom_top(Heap *heap, int index)
{
    Instance temp;
    int parent_node = (index - 1) / 2;

    int new_val = heap->instances[index].weight;
    int parent_val = heap->instances[parent_node].weight;

    if (parent_val > new_val)
    {

        temp = heap->instances[parent_node];
        heap->instances[parent_node] = heap->instances[index];
        heap->instances[index] = temp;
        heapify_bottom_top(heap, parent_node);

        return;
    }
}

void heapify_top_bottom(Heap *heap, int parent_node)
{
    int left = parent_node * 2 + 1;
    int right = parent_node * 2 + 2;
    int min;
    Instance temp;

    if (left >= heap->count || left < 0)
        left = -1;
    if (right >= heap->count || right < 0)
        right = -1;

    if (left != -1 && heap->instances[left].weight < heap->instances[parent_node].weight)
        min = left;
    else
        min = parent_node;
    if (right != -1 && heap->instances[right].weight < heap->instances[min].weight)
        min = right;

    if (min != parent_node)
    {
        temp = heap->instances[min];
        heap->instances[min] = heap->instances[parent_node];
        heap->instances[parent_node] = temp;

        heapify_top_bottom(heap, min);
    }
}

void pop_min(Heap *heap)
{
    // Instance _pop;
    if (heap->count == 0)
    {
        printf("\n__Heap is Empty__\n");
        return;
    }

    // _pop = heap->instances[0];
    heap->instances[0] = heap->instances[heap->count - 1];
    heap->count--;
    heapify_top_bottom(heap, 0);
    return;
}

#endif