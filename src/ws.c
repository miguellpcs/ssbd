#include <stdio.h>

#include "../lib/array.h"
#include "../lib/instance.h"
#include "../lib/heap.h"

int uniform_distribution(int, int);

typedef struct ws
{
    Heap *high;
    Heap *low;
    float tal;
    int count;
    int limit;
    int len;
} WS;
WS *init(int);
WS *update(WS *, Instance);
void query(WS *);

int main()
{
    int tst_size = 100;
    int random_sample_size = 10;

    Array *tst = create_sample(tst_size);

    printf("Stream:\n");
    print_sample(tst);
    printf("Random Sample size: %d \n", random_sample_size);

    WS *sketch = init(random_sample_size);

    for (int i = 0; i < tst_size; i++)
    {
        sketch = update(sketch, tst->data[i]);
    }
    printf("Random Sample:\n");
    query(sketch);

    return 0;
}

// MARK - Weighted Sampling functions
WS *init(int size)
{
    WS *sketch = malloc(sizeof(WS));
    sketch->count = 0;
    sketch->len = 0;
    sketch->tal = 0;
    sketch->limit = size;
    //sketch->data = malloc(size*sizeof(int));

    sketch->high = malloc(sizeof(Heap));
    sketch->high->instances = malloc(size * sizeof(Instance));
    sketch->high->count = 0;

    sketch->low = malloc(sizeof(Heap));
    sketch->low->instances = NULL;
    sketch->low->count = 0;

    return sketch;
}

WS *update(WS *sketch, Instance x)
{ // Update fors simple RS. TODO: add tal update
    if (sketch->len < sketch->limit)
    {
        insert_min_heap(sketch->high, &x);
        sketch->count += 1;
        sketch->len += 1;
    }

    else
    {
        sketch->count += 1;
        int i = uniform_distribution(0, sketch->count);
        if (i < sketch->limit)
        {
            pop_min(sketch->high);
            insert_min_heap(sketch->high, &x);
        }
    }

    return sketch;
}

void query(WS *sketch)
{
    for (int i = 0; i < sketch->len; i++)
    {
        printf("val = %d, weight = %d \n ", sketch->high->instances[i].val, sketch->high->instances[i].weight);
    }
    printf("\n");
}
